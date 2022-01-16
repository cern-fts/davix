/*
   Copyright (c) 2002-7, Andrew McNab, University of Manchester
   All rights reserved.
   Redistribution and use in source and binary forms, with or
   without modification, are permitted provided that the following
   conditions are met:
   o Redistributions of source code must retain the above
     copyright notice, this list of conditions and the following
     disclaimer.
   o Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the following
     disclaimer in the documentation and/or other materials
     provided with the distribution.
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
   CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
   INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------
    For more information about GridSite: http://www.gridsite.org/
   ---------------------------------------------------------------
*/

#include <string.h>

#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/pem.h>

#include "GRSTx509MakeProxyCert.h"

#define GRST_BACKDATE_SECONDS   300
#define GRST_PROXYCERTINFO_OID  "1.3.6.1.5.5.7.1.14"
#define GRST_KEYUSAGE_OID       "2.5.29.15"


static void mpcerror(FILE *debugfp, const char *msg)
{
  if (debugfp != NULL)
    {
      fputs(msg, debugfp);
      ERR_print_errors_fp(debugfp);
    }
}


static time_t GRSTasn1TimeToTimeT(const unsigned char *asn1time, size_t len)
{
   char   zone;
   struct tm time_tm;

   if (len == 0) len = strlen((const char*)asn1time);

   if ((len != 13) && (len != 15)) return 0; /* dont understand */

   if ((len == 13) &&
       ((sscanf((const char*)asn1time, "%02d%02d%02d%02d%02d%02d%c",
         &(time_tm.tm_year),
         &(time_tm.tm_mon),
         &(time_tm.tm_mday),
         &(time_tm.tm_hour),
         &(time_tm.tm_min),
         &(time_tm.tm_sec),
         &zone) != 7) || (zone != 'Z'))) return 0; /* dont understand */

   if ((len == 15) &&
       ((sscanf((const char*)asn1time, "20%02d%02d%02d%02d%02d%02d%c",
         &(time_tm.tm_year),
         &(time_tm.tm_mon),
         &(time_tm.tm_mday),
         &(time_tm.tm_hour),
         &(time_tm.tm_min),
         &(time_tm.tm_sec),
         &zone) != 7) || (zone != 'Z'))) return 0; /* dont understand */

   /* time format fixups */

   if (time_tm.tm_year < 90) time_tm.tm_year += 100;
   --(time_tm.tm_mon);

   return timegm(&time_tm);
}


#if OPENSSL_VERSION_NUMBER < 0x1000000fL
int EVP_PKEY_base_id(const EVP_PKEY *pkey)
{
    return EVP_PKEY_type(pkey->type);
}
#endif

int GRSTx509MakeProxyCert(char **proxychain, FILE *debugfp,
                          char *reqtxt, char *cert, char *key, int minutes)
///
/// The proxy chain is returned in *proxychain. If debugfp is non-NULL,
/// errors are output to that file pointer. The proxy will expired in
/// the given number of minutes starting from the current time.
{
  char *ptr, *certchain, s[41];
  static unsigned char pci_str[] = { 0x30, 0x0c, 0x30, 0x0a, 0x06, 0x08,
    0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x15, 0x01, 0 },
    kyu_str[] = { 0x03, 0x02, 0x03,
                  X509v3_KU_DIGITAL_SIGNATURE |
                  X509v3_KU_KEY_ENCIPHERMENT  |
                  X509v3_KU_KEY_AGREEMENT,
                  0 };
  int i, ncerts, any_rfc_proxies = 0;
  long ptrlen;
  EVP_PKEY *pkey, *CApkey;
  const EVP_MD *digest;
  X509 **certs = NULL;
  X509_REQ *req;
  X509_NAME *name, *CAsubject, *newsubject;
  X509_NAME_ENTRY *ent;
  ASN1_OBJECT *pci_obj = NULL, *kyu_obj;
  ASN1_OCTET_STRING *pci_oct, *kyu_oct;
  X509_EXTENSION *pci_ex, *kyu_ex;
  FILE *fp;
  BIO *reqmem, *certmem;
  time_t notAfter;

  /* read in the request */
  reqmem = BIO_new(BIO_s_mem());
  BIO_puts(reqmem, reqtxt);

  if (!(req = PEM_read_bio_X509_REQ(reqmem, NULL, NULL, NULL)))
    {
      mpcerror(debugfp,
              "GRSTx509MakeProxyCert(): error reading request from BIO memory\n");
      BIO_free(reqmem);
      return GRST_RET_FAILED;
    }

  BIO_free(reqmem);

  /* verify signature on the request */
  if (!(pkey = X509_REQ_get_pubkey(req)))
    {
      mpcerror(debugfp,
              "GRSTx509MakeProxyCert(): error getting public key from request\n");

      X509_REQ_free(req);
      return GRST_RET_FAILED;
    }

  if (X509_REQ_verify(req, pkey) != 1)
    {
      mpcerror(debugfp,
            "GRSTx509MakeProxyCert(): error verifying signature on certificate\n");

      X509_REQ_free(req);
      return GRST_RET_FAILED;
    }

  /* read in the signing certificate */
  if (!(fp = fopen(cert, "r")))
    {
      mpcerror(debugfp,
            "GRSTx509MakeProxyCert(): error opening signing certificate file\n");

      X509_REQ_free(req);
      return GRST_RET_FAILED;
    }

  ncerts = 1;
  while (1)
  {
   certs = (X509 **) realloc(certs, (sizeof(X509 *)) * (ncerts+1));
   if (certs == NULL)
       mpcerror(debugfp,
               "GRSTx509MakeProxyCert(): no memory\n");
   if ((certs[ncerts] = PEM_read_X509(fp, NULL, NULL, NULL)) == NULL)
       break;
   ncerts++;

  }

  if (ncerts == 1) /* zeroth cert with be new proxy cert */
    {
      mpcerror(debugfp,
            "GRSTx509MakeProxyCert(): error reading signing certificate file\n");

      X509_REQ_free(req);
      return GRST_RET_FAILED;
    }

  fclose(fp);
  fp = NULL;

  CAsubject = X509_get_subject_name(certs[1]);

  /* read in the CA private key */
  if (!(fp = fopen(key, "r")))
    {
      mpcerror(debugfp,
            "GRSTx509MakeProxyCert(): error reading signing private key file\n");

      X509_REQ_free(req);
      return GRST_RET_FAILED;
    }

  if (!(CApkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL)))
    {
      mpcerror(debugfp,
            "GRSTx509MakeProxyCert(): error reading signing private key in file\n");

      X509_REQ_free(req);
      return GRST_RET_FAILED;
    }

  fclose(fp);

  /* get subject name */
  if (!(name = X509_REQ_get_subject_name(req)))
    {
      mpcerror(debugfp,
            "GRSTx509MakeProxyCert(): error getting subject name from request\n");

      X509_REQ_free(req);
      return GRST_RET_FAILED;
    }

  /* create new certificate */
  if (!(certs[0] = X509_new()))
    {
      mpcerror(debugfp,
            "GRSTx509MakeProxyCert(): error creating X509 object\n");

      X509_REQ_free(req);
      return GRST_RET_FAILED;
    }

  /* set version number for the certificate (X509v3) and the serial number

     We now use 2 = v3 for the GSI proxy, rather than the old Globus
     behaviour of 3 = v4. See Savannah Bug #53721 */

  if (X509_set_version(certs[0], 2L) != 1)
    {
      mpcerror(debugfp,
            "GRSTx509MakeProxyCert(): error setting certificate version\n");

      X509_REQ_free(req);
      return GRST_RET_FAILED;
    }

  ASN1_INTEGER_set(X509_get_serialNumber(certs[0]), (long) time(NULL));

  if (!(name = X509_get_subject_name(certs[1])))
    {
      mpcerror(debugfp,
      "GRSTx509MakeProxyCert(): error getting subject name from CA certificate\n");

      X509_REQ_free(req);
      return GRST_RET_FAILED;
    }

  if (X509_set_issuer_name(certs[0], name) != 1)
    {
      mpcerror(debugfp,
      "GRSTx509MakeProxyCert(): error setting issuer name of certificate\n");

      X509_REQ_free(req);
      return GRST_RET_FAILED;
    }

  /* set public key in the certificate */
  if (X509_set_pubkey(certs[0], pkey) != 1)
    {
      mpcerror(debugfp,
      "GRSTx509MakeProxyCert(): error setting public key of the certificate\n");

      X509_REQ_free(req);
      return GRST_RET_FAILED;
    }

  /* set duration for the certificate */
  if (!(X509_gmtime_adj(X509_get_notBefore(certs[0]), -GRST_BACKDATE_SECONDS)))
    {
      mpcerror(debugfp,
      "GRSTx509MakeProxyCert(): error setting beginning time of the certificate\n");

      X509_REQ_free(req);
      return GRST_RET_FAILED;
    }

  if (!(X509_gmtime_adj(X509_get_notAfter(certs[0]), 60 * minutes)))
    {
      mpcerror(debugfp,
      "GRSTx509MakeProxyCert(): error setting ending time of the certificate\n");

      X509_REQ_free(req);
      return GRST_RET_FAILED;
    }

  /* go through chain making sure this proxy is not longer lived */

  pci_obj = OBJ_txt2obj(GRST_PROXYCERTINFO_OID, 0);

  notAfter =
     GRSTasn1TimeToTimeT(ASN1_STRING_data(X509_get_notAfter(certs[0])), 0);

  for (i=1; i < ncerts; ++i)
     {
       if (notAfter >
           GRSTasn1TimeToTimeT(ASN1_STRING_data(X509_get_notAfter(certs[i])),
                               0))
         {
           notAfter =
            GRSTasn1TimeToTimeT(ASN1_STRING_data(X509_get_notAfter(certs[i])),
                                0);

           ASN1_UTCTIME_set(X509_get_notAfter(certs[0]), notAfter);
         }

       if (X509_get_ext_by_OBJ(certs[i], pci_obj, -1) > 0)
         any_rfc_proxies = 1;
     }

   /* if any earlier proxies are RFC 3820, then new proxy must be
      an RFC 3820 proxy too with the required extensions */
   if (any_rfc_proxies)
    {
      /* key usage */
      kyu_obj = OBJ_txt2obj(GRST_KEYUSAGE_OID, 0);
      kyu_ex = X509_EXTENSION_new();

      X509_EXTENSION_set_object(kyu_ex, kyu_obj);
      ASN1_OBJECT_free(kyu_obj);
      X509_EXTENSION_set_critical(kyu_ex, 1);

      kyu_oct = ASN1_OCTET_STRING_new();
      ASN1_OCTET_STRING_set(kyu_oct, kyu_str, strlen((const char*)kyu_str));
      X509_EXTENSION_set_data(kyu_ex, kyu_oct);
      ASN1_OCTET_STRING_free(kyu_oct);

      X509_add_ext(certs[0], kyu_ex, -1);
      X509_EXTENSION_free(kyu_ex);

      /* proxy certificate info */
      pci_ex = X509_EXTENSION_new();

      X509_EXTENSION_set_object(pci_ex, pci_obj);
      X509_EXTENSION_set_critical(pci_ex, 1);

      pci_oct = ASN1_OCTET_STRING_new();
      ASN1_OCTET_STRING_set(pci_oct, pci_str, strlen((const char*)pci_str));
      X509_EXTENSION_set_data(pci_ex, pci_oct);
      ASN1_OCTET_STRING_free(pci_oct);

      X509_add_ext(certs[0], pci_ex, -1);
      X509_EXTENSION_free(pci_ex);
    }
  ASN1_OBJECT_free(pci_obj);

  /* set issuer and subject name of the cert from the req and the CA */

  if (any_rfc_proxies) /* user CN=number rather than CN=proxy */
    {
       snprintf(s, sizeof(s), "%ld", (long) time(NULL));
       ent = X509_NAME_ENTRY_create_by_NID(NULL, OBJ_txt2nid("commonName"),
                                      MBSTRING_ASC, (unsigned char*)s, -1);
    }
  else ent = X509_NAME_ENTRY_create_by_NID(NULL, OBJ_txt2nid("commonName"),
                                      MBSTRING_ASC, (unsigned char*)"proxy", -1);

  newsubject = X509_NAME_dup(CAsubject);

  X509_NAME_add_entry(newsubject, ent, -1, 0);

  if (X509_set_subject_name(certs[0], newsubject) != 1)
    {
      mpcerror(debugfp,
      "GRSTx509MakeProxyCert(): error setting subject name of certificate\n");

      X509_REQ_free(req);
      return GRST_RET_FAILED;
    }

  X509_NAME_free(newsubject);
  X509_NAME_ENTRY_free(ent);

  /* sign the certificate with the signing private key */
  if (EVP_PKEY_base_id(CApkey) == EVP_PKEY_RSA)
    {
        digest = EVP_sha512();
    }
  else
    {
      mpcerror(debugfp,
      "GRSTx509MakeProxyCert(): error checking signing private key for a valid digest\n");

      X509_REQ_free(req);
      return GRST_RET_FAILED;
    }

  if (!(X509_sign(certs[0], CApkey, digest)))
    {
      mpcerror(debugfp,
      "GRSTx509MakeProxyCert(): error signing certificate\n");

      X509_REQ_free(req);
      return GRST_RET_FAILED;
    }

  /* store the completed certificate chain */

  certchain = strdup("");

  for (i=0; i < ncerts; ++i)
     {
       certmem = BIO_new(BIO_s_mem());

       if (PEM_write_bio_X509(certmem, certs[i]) != 1)
         {
           mpcerror(debugfp,
            "GRSTx509MakeProxyCert(): error writing certificate to memory BIO\n");

           X509_REQ_free(req);
           return GRST_RET_FAILED;
         }

       ptrlen = BIO_get_mem_data(certmem, &ptr);

       certchain = (char*)realloc(certchain, strlen((const char*)certchain) + ptrlen + 1);

       strncat(certchain, ptr, ptrlen);

       BIO_free(certmem);
       X509_free(certs[i]);
     }

  if (certs)
      free (certs);
  EVP_PKEY_free(pkey);
  EVP_PKEY_free(CApkey);
  X509_REQ_free(req);

  *proxychain = certchain;
  return GRST_RET_OK;
}
