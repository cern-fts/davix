#include <cstring>
#include <iostream>
#include "test_credential.h"
#include <libs/security/crypto_utils.h>
#include <glibmm.h>
#include <glib.h>

const char* private_key= "-----BEGIN PRIVATE KEY-----\n"
"MIICdQIBADANBgkqhkiG9w0BAQEFAASCAl8wggJbAgEAAoGBANB9fCqUBCoWYA5x\n"
"4NSr9DmkO2q9CCCTciBmOcgK6glJbnzSXu6RFZCO9DYrE8vFoyIcsm8RtoqV5RX4\n"
"wSwADNWohaWlld7ZOC7hGI80HpliP2t5wOUEJEh3UTr01hiYUuqa/CmPhYe3z51r\n"
"MrlwuQ5YNOzsygEGwDQOoyGMjxDbAgMBAAECgYBNdeHU++tUK74Cf+LVgRGa/N6q\n"
"eJL1b3KegPyEyzFKxAKN06c7oaHOdJ5dJcIUGljSyrCsvXsBoFFdyW8Txuz9JEbi\n"
"QukXylYkVyTyXsviKUNdteTfNYPMNUs9DQ/eeHmCB+qlQfGSg0MYCpLw8dqhsbeO\n"
"ZU0zZn9jmmM92qo3mQJBAP6EGlqR4EHc1dYKQyiaOVEmGUY5WqDYzhZkLpGPRlYS\n"
"p9kybAj1Eb2QNyfFPVXsetT8Kk5yoh6OyekgfWgLioUCQQDRtK7HeVeIhcjq0/4j\n"
"/ZFqu91JC4eLiqBwAjAe+C7kX6uuFrBYfqVNQhEYQh7ZgKPTiKnsU4PrLmAGK524\n"
"PfvfAkAW9aPK983bIyjHHjXgu8jf4Sf2hcX/LI+qxW7OAra9nQE3Pq0wQatQBmpz\n"
"U/+seKC8BeNrIDwvtYHRXfTyJdhhAkBmmuNRkltz3xG5ZFRaw7yc3qKdFNkTO5bY\n"
"dxmYbZJ0ByN3IH7ULdMvg/3dQqzZewmtfJa3nP1U2vH80uZuuVU9AkA1c//oheuF\n"
"OT+VkN5Vn164rg+uZsTG9IIbTDa70TXygOnojfLHgf/t2DVbje+h3zskgEaJpawf\n"
"wECXTKob+HMH\n"
"-----END PRIVATE KEY-----\n";

const char* cert_key= "-----BEGIN CERTIFICATE----- \n"
"MIICdjCCAd+gAwIBAgIJAIRewOXKoOkRMA0GCSqGSIb3DQEBBQUAMFQxCzAJBgNV\n"
"BAYTAkZSMQ8wDQYDVQQIDAZGcmFuY2UxETAPBgNVBAcMCE5vd3doZXJlMSEwHwYD\n"
"VQQKDBhJbnRlcm5ldCBXaWRnaXRzIFB0eSBMdGQwHhcNMTIwMzE2MjEwMzIwWhcN\n"
"MTMwMzE2MjEwMzIwWjBUMQswCQYDVQQGEwJGUjEPMA0GA1UECAwGRnJhbmNlMREw\n"
"DwYDVQQHDAhOb3d3aGVyZTEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQdHkg\n"
"THRkMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDQfXwqlAQqFmAOceDUq/Q5\n"
"pDtqvQggk3IgZjnICuoJSW580l7ukRWQjvQ2KxPLxaMiHLJvEbaKleUV+MEsAAzV\n"
"qIWlpZXe2Tgu4RiPNB6ZYj9recDlBCRId1E69NYYmFLqmvwpj4WHt8+dazK5cLkO\n"
"WDTs7MoBBsA0DqMhjI8Q2wIDAQABo1AwTjAdBgNVHQ4EFgQUydrZH7319wywybtH\n"
"z9KZy1B0AkowHwYDVR0jBBgwFoAUydrZH7319wywybtHz9KZy1B0AkowDAYDVR0T\n"
"BAUwAwEB/zANBgkqhkiG9w0BAQUFAAOBgQCXsSFeFJx0tUMbou3jF/JieVcqmYkS\n"
"7+GT2harl0y/wjyok+lIdHcAQ6TuXFflYriC1c3d+RPoWA116Q01OnUDXDTSt5J5\n"
"ORwDQvWWG13zAwymbMs8Di8t4UkOAz0ECf9ulO64iC+8wdYPQR7G7H7i0X2BG5tK\n"
"kHJ18dhOkgi4nA==\n"
"-----END CERTIFICATE-----\n";



const char* false_cert_key= "-----BEGIN CERTIFICATE----- \n"
"MIICdjCCAd+gAwIBAgIJAIRewOXKoOkRMA0GCSqGSIb3DQEBBQUAMFQxCzAJBgNV\n"
"BAYTAkZSMQ8wDQfdsfsdfdsGcmFuY2UxETAPBgNVBAcMCE5vd3doZXJlMSEwHwYD\n"
"VQQKDBhJbnRlcm5ldCBXaWRnaXRzIFB0eSBMdGQwHhcNMTIwMzE2MjEwMzIwWhcN\n"
"MTMwMzE2MjEwMzIwWjBUMQswCQYDVQQGEwJGUjEPMA0GA1UECAwGRnJhbmNlMREw\n"
"DwYDVQQHDAhOb3d3aGVyZTEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQdHkg\n"
"THRkMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDQfXwqlAQqFmAOceDUq/Q5\n"
"pDtqvQggk3IgZjnICuoJSW580l7ukRWQjvQ2KxPLxaMiHLJvEbaKleUV+MEsAAzV\n"
"qIWlpZXe2Tgu4RiPNB6ZYj9recDlBCRId1E69NYYmFLqmvwpj4WHt8+dazK5cLkO\n"
"WDTs7MoBBsA0DqMhjI8Q2wIDAQABo1AwTjAdBgNVHQ4EFgQUydrZH7319wywybtH\n"
"z9KZy1B0AkowHwYDVR0jBBgwFoAUydrZH7319wywybtHz9KZy1B0AkowDAYDVR0T\n"
"BAUwAwEB/zANBgkqhkiG9w0BAQUFAAOBgQCXsSFeFJx0tUMbou3jF/JieVcqmYkS\n"
"7+GT2harl0y/wjyok+lIdHcAQ6TuXFflYriC1c3d+RPoWA116Q01OnUDXDTSt5J5\n"
"ORwDQvWWG13zAwymbMs8Di8t4UkOAz0ECf9ulO64iC+8wdYPQR7G7H7i0X2BG5tK\n"
"kHJ18dhOkgi4nA==\n"
"-----END CERTIFICATE-----\n";

const char* proxy_cert ="-----BEGIN CERTIFICATE-----\n"
"MIINeDCCDGCgAwIBAgIEAf1d0DANBgkqhkiG9w0BAQUFADCBkDESMBAGCgmSJomT\n"
"8ixkARkWAmNoMRQwEgYKCZImiZPyLGQBGRYEY2VybjEWMBQGA1UECxMNT3JnYW5p\n"
"YyBVbml0czEOMAwGA1UECxMFVXNlcnMxETAPBgNVBAMTCGFkZXZyZXNzMQ8wDQYD\n"
"VQQDEwY3MTQ0MjUxGDAWBgNVBAMTD0FkcmllbiBEZXZyZXNzZTAeFw0xMjAzMTcx\n"
"NzA3MTNaFw0xMjAzMTgwNTEyMTNaMIGgMRIwEAYKCZImiZPyLGQBGRYCY2gxFDAS\n"
"BgoJkiaJk/IsZAEZFgRjZXJuMRYwFAYDVQQLEw1PcmdhbmljIFVuaXRzMQ4wDAYD\n"
"VQQLEwVVc2VyczERMA8GA1UEAxMIYWRldnJlc3MxDzANBgNVBAMTBjcxNDQyNTEY\n"
"MBYGA1UEAxMPQWRyaWVuIERldnJlc3NlMQ4wDAYDVQQDEwVwcm94eTCBnzANBgkq\n"
"hkiG9w0BAQEFAAOBjQAwgYkCgYEAxROV60uKqQgAkHLUDYz90k5Tns1FzD8aYzNW\n"
"e7lGtLDFLaZfKCrbY7iZvlpbXGDNlNkIZ/Q9QWbLu1kdney2QHpjrRes2JhtiJTG\n"
"Ho6UaNu1jZ9upa1Em7+GklZQEjh3CiDv3VE93KAttuR0Hk1fmbYGhDAFgKfMUz1I\n"
"t3t6MmECAwEAAaOCCkowggpGMIIKIAYKKwYBBAG+RWRkBQSCChAwggoMMIIKCDCC\n"
"CgQwggjsAgEBMIGooIGlMIGWpIGTMIGQMRIwEAYKCZImiZPyLGQBGRYCY2gxFDAS\n"
"BgoJkiaJk/IsZAEZFgRjZXJuMRYwFAYDVQQLEw1PcmdhbmljIFVuaXRzMQ4wDAYD\n"
"VQQLEwVVc2VyczERMA8GA1UEAxMIYWRldnJlc3MxDzANBgNVBAMTBjcxNDQyNTEY\n"
"MBYGA1UEAxMPQWRyaWVuIERldnJlc3NlAgpZX232AAIAAPwIoGAwXqRcMFoxEjAQ\n"
"BgoJkiaJk/IsZAEZFgJjaDEUMBIGCgmSJomT8ixkARkWBGNlcm4xEjAQBgNVBAsT\n"
"CWNvbXB1dGVyczEaMBgGA1UEAxMRbHhicmEyMzA5LmNlcm4uY2gwDQYJKoZIhvcN\n"
"AQEEBQACEQDGFcSlnSpOTYPMoHN7ibdNMCIYDzIwMTIwMzE3MTcxMjEzWhgPMjAx\n"
"MjAzMTgwNTEyMTNaMFkwVwYKKwYBBAG+RWRkBDFJMEegIYYfZHRlYW06Ly9seGJy\n"
"YTIzMDkuY2Vybi5jaDoxNTAwMjAiBCAvZHRlYW0vUm9sZT1OVUxML0NhcGFiaWxp\n"
"dHk9TlVMTDCCBzcwEgYKKwYBBAG+RWRkCwQEMAIwADAJBgNVHTgEAgUAMB8GA1Ud\n"
"IwQYMBaAFBvVMU416+aaP6SU0Zlwps3wxOFDMIIG8wYKKwYBBAG+RWRkCgSCBuMw\n"
"ggbfMIIG2zCCBtcwggW/oAMCAQICChtP3P4AAgAAm5IwDQYJKoZIhvcNAQEFBQAw\n"
"WTESMBAGCgmSJomT8ixkARkWAmNoMRQwEgYKCZImiZPyLGQBGRYEY2VybjEtMCsG\n"
"A1UEAxMkQ0VSTiBUcnVzdGVkIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MB4XDTEx\n"
"MDYwNzA5NTI0NFoXDTEyMDYwNjA5NTI0NFowWjESMBAGCgmSJomT8ixkARkWAmNo\n"
"MRQwEgYKCZImiZPyLGQBGRYEY2VybjESMBAGA1UECxMJY29tcHV0ZXJzMRowGAYD\n"
"VQQDExFseGJyYTIzMDkuY2Vybi5jaDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCC\n"
"AQoCggEBAM0tilENvScUO8K5yeFzExrgw/HY0dfSoOdSHeJkc6sYeKrzlLEUkq0s\n"
"Ezzup0N23e+3OZRZZrlWVfRsGjYpmrxLRwwQksPeuQmVoSKaZdrjb761q1y3noUT\n"
"iAZ+Y52TGG5Itj8v3KDYAcjkJfVnaLa/Dl8Iou11WYUkAGn/HZkEOAN615kxyB0w\n"
"7sWzejx/+K9VGN8VUWDYghxYonjnsFeAfcSc4Fai1RyO8fOfrWiwqzF9+E9Mr8Ja\n"
"xX7NYz6BRGJcb/DP9ZWR/XuLi/oIYEJ9qRwJ9e+V6d3mG0KdSMUt6Fc6RCubKdzB\n"
"CgvD0pR4sHXXpeYGIQjjfo/KV/t1T8kCAwEAAaOCA54wggOaMB0GA1UdDgQWBBSP\n"
"zBmJBNWoQldb/xnqLEmmMUtB9DAfBgNVHSMEGDAWgBSYzJLQRjA2jLDtmA1yUalH\n"
"TN2+ITCCATQGA1UdHwSCASswggEnMIIBI6CCAR+gggEbhkdodHRwOi8vY2EuY2Vy\n"
"bi5jaC9jYS9jcmwvQ0VSTiUyMFRydXN0ZWQlMjBDZXJ0aWZpY2F0aW9uJTIwQXV0\n"
"aG9yaXR5LmNybIaBz2xkYXA6Ly8vQ049Q0VSTiUyMFRydXN0ZWQlMjBDZXJ0aWZp\n"
"Y2F0aW9uJTIwQXV0aG9yaXR5LENOPWNlcm5wa2kwMSxDTj1DRFAsQ049UHVibGlj\n"
"JTIwS2V5JTIwU2VydmljZXMsQ049U2VydmljZXMsQ049Q29uZmlndXJhdGlvbixE\n"
"Qz1jZXJuLERDPWNoP2NlcnRpZmljYXRlUmV2b2NhdGlvbkxpc3Q/YmFzZT9vYmpl\n"
"Y3RDbGFzcz1jUkxEaXN0cmlidXRpb25Qb2ludDCCAUQGCCsGAQUFBwEBBIIBNjCC\n"
"ATIwaAYIKwYBBQUHMAKGXGh0dHA6Ly9jYS5jZXJuLmNoL2NhL2NybC9jZXJucGtp\n"
"MDEuY2Vybi5jaF9DRVJOJTIwVHJ1c3RlZCUyMENlcnRpZmljYXRpb24lMjBBdXRo\n"
"b3JpdHkoMikuY3J0MIHFBggrBgEFBQcwAoaBuGxkYXA6Ly8vQ049Q0VSTiUyMFRy\n"
"dXN0ZWQlMjBDZXJ0aWZpY2F0aW9uJTIwQXV0aG9yaXR5LENOPUFJQSxDTj1QdWJs\n"
"aWMlMjBLZXklMjBTZXJ2aWNlcyxDTj1TZXJ2aWNlcyxDTj1Db25maWd1cmF0aW9u\n"
"LERDPWNlcm4sREM9Y2g/Y0FDZXJ0aWZpY2F0ZT9iYXNlP29iamVjdENsYXNzPWNl\n"
"cnRpZmljYXRpb25BdXRob3JpdHkwDAYDVR0TAQH/BAIwADAOBgNVHQ8BAf8EBAMC\n"
"BaAwPQYJKwYBBAGCNxUHBDAwLgYmKwYBBAGCNxUIg73QCYLtjQ2G7Ysrgd71N4WA\n"
"0GIehtLMUIHZ81sCAWQCAQkwHQYDVR0lBBYwFAYIKwYBBQUHAwIGCCsGAQUFBwMB\n"
"MDMGA1UdIAQsMCowKAYmKwYBBAGCNxUIg73QCYLtjQ2G7Ysrgd71N4WA0GIeg6yh\n"
"UoTb1CEwJwYJKwYBBAGCNxUKBBowGDAKBggrBgEFBQcDAjAKBggrBgEFBQcDATAN\n"
"BgkqhkiG9w0BAQUFAAOCAQEArvIknrxsKyqXsBY/cGW48RVlOp/bjrYL5GZD4dZD\n"
"G80b5xSYezYSPAsT7nOjhRNxSj/BD4id1765AfA9O0Lq3CovSqGol8s3SwsEki+h\n"
"wklsBSeVam0dGWiGSeoMWaEZsGAaBu3Gm165w/SJ29Ttm8mHqxcQKAyfX6sRD8yH\n"
"54rsYe7orfGYOlZ1pyQW4hp1IAEoU/YjPWaQcKHkhPsbEhH/ANVCXxFFxTsgov6O\n"
"N86oaoexqOuPjTAHAkRaQiEANSz/RxDi8kxQfQKElU/uSMjq1vMKqoI7+DLuQhzy\n"
"pZh/eh8l48L9+6QT7BoWHNhbi6Knv90n0fwwqJRWSz9YqDANBgkqhkiG9w0BAQQF\n"
"AAOCAQEAihyUZ72OlclY2jPKJi1FhJVieilnnbwfQ+8WuiyqP9OfI/Q8gr6ELFlr\n"
"vE9HSrtnSunx+PxKQKBUtyCTLhymoks/gyHD5gKj10d56mIUAYCwBVZooONg5TIF\n"
"htxxyugIMX+0p+EbhHKALk2cBs4WkZmrooG+B+0OhgLkLKHzQOqnbLtUILbw+jZS\n"
"3ddr4emOHf0Hk+Oj7UtKD8uvsFuKSBFvNpDpi7B3IDcYCKqzz5cBhkDa8UbjuFFA\n"
"2ddU10LyGGUd92J03YLmxltUddtFUBhw/jxLVnMrptX3cNahKPS5Vx5ztcspjFBi\n"
"Uq5kb+3glvzfscwzKHcO1iuYX8ApajAOBgNVHQ8BAf8EBAMCBLAwEAYKKwYBBAG+\n"
"RWRkBgQCMDMwDQYJKoZIhvcNAQEFBQADggEBADApDWI6l7nHQS3TihmboCLQubaH\n"
"m+rHtxr5rLiXvL3zy5dkYNV4zIaEz/ZxLnjEne8Q0Io7gmPa6FTEFECryg3ar5LB\n"
"YyTZaiWDu29H/iNZO3k0WeY1UWEqhVL+BFgIwhLVVLEBdPCjX8xk9vR5xZbQuWco\n"
"Tz5nDeqkyYE/mLJpMaT8ef2qqaMSsBKy9e0v74kAaSoVICl5bgx9MB+H5+NloEQx\n"
"seUFz0QFCpC9ay0zv4srgTabK2fPX30VZpvt5iNLE8P+84yvlTuUFmsAQAah3l1+\n"
"+LisIDgEMfzNxgblx7UTY/Y+KwO/TLuZz0AR3pc/fufgxJ80ODqz9NBLUKo=\n"
"-----END CERTIFICATE-----\n"
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIICXQIBAAKBgQDFE5XrS4qpCACQctQNjP3STlOezUXMPxpjM1Z7uUa0sMUtpl8o\n"
"KttjuJm+WltcYM2U2Qhn9D1BZsu7WR2d7LZAemOtF6zYmG2IlMYejpRo27WNn26l\n"
"rUSbv4aSVlASOHcKIO/dUT3coC225HQeTV+ZtgaEMAWAp8xTPUi3e3oyYQIDAQAB\n"
"AoGBAMCT7/bafafJOwvsOYz4Tnu4sHvIaUE6FKYFX6hNF2uwJS4Dmo7cuj/K/umE\n"
"0CwIKDwgjNdQUp9bPRBY99j/m+LUd2Ab3fAqSdwQ1qYSs3OsHZrtMJN9NecKoxnM\n"
"lkWSH8GB44J1YHtLYJrYkovy+wDzZV211bwtseUU0bBuza2xAkEA68CYugRgW9A4\n"
"m7DHZSHwPVtUz66K7Qa3V4B6ckHi31E1ffnAhL17Mm7fQT7YI4+ynwWLpUeyLrHC\n"
"YRgATKrvnQJBANYAoydF0hw8MnsoPJn9LD3A0e0fH/n4oa7UcoevsV8REcNI26xK\n"
"VSAJIT3eEV78enlNtZN4VcG1ftlLBpP47JUCQERmyilWsbqR7IrHvOLL+Q9kW3Qy\n"
"mV5yT2nU/jH+idvvjQyzFTeuXntgjeg5Wq7et53KFx1qcvl3XAWd2CBmjEUCQCAn\n"
"clw5QmuZo+AbWJeukZIpwaEGNzDA6dIx+49ll5n4H5oe/VqyxH2OwZ7hGe0StHg7\n"
"c9fXkdMMKSYO/ssG8M0CQQCcj9eDEwjKrAaPLbPC0E7T9EFzyB/8Ep7w6I+l0aEj\n"
"lyApGmy10T9oOWNg9DW88UVDwV9wA4vvoVItZPskN7Uw\n"
"-----END RSA PRIVATE KEY-----\n"
"-----BEGIN CERTIFICATE-----\n"
"MIIHejCCBmKgAwIBAgIKWV9t9gACAAD8CDANBgkqhkiG9w0BAQUFADBZMRIwEAYK\n"
"CZImiZPyLGQBGRYCY2gxFDASBgoJkiaJk/IsZAEZFgRjZXJuMS0wKwYDVQQDEyRD\n"
"RVJOIFRydXN0ZWQgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTIwMjIzMDc1\n"
"OTM4WhcNMTMwMjIyMDc1OTM4WjCBkDESMBAGCgmSJomT8ixkARkWAmNoMRQwEgYK\n"
"CZImiZPyLGQBGRYEY2VybjEWMBQGA1UECxMNT3JnYW5pYyBVbml0czEOMAwGA1UE\n"
"CxMFVXNlcnMxETAPBgNVBAMTCGFkZXZyZXNzMQ8wDQYDVQQDEwY3MTQ0MjUxGDAW\n"
"BgNVBAMTD0FkcmllbiBEZXZyZXNzZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCC\n"
"AQoCggEBAMgO/5dx4gNZlX6Vt4K3axmvat0WSg7fuhvgiAOqqYlNIa5pQaJGbqni\n"
"p/RC/ZckG+hX4xW76csm5pDFCLS1zKzHvBjH3kGzHQZyUlRM8PjMs2h/YC4jV+yz\n"
"zMjbEGSp35M3OUkzx60QbfPEVN3kwzQx01QeN8QJVL1aTOZWZMqiaH5JXgXlI49g\n"
"xThHE+gdyoawzfFcS7l/YpjJNHy28McyNs8okAk+79+em/P2UPvGG1pyw1hCNmeB\n"
"NVKSi4j+M8ONrgDJa3OCkg4o85upqlX9sAk7C/pKsvB1uMfy1KQZYNEsyZMm2ooK\n"
"2Rwj1s0w8JvB/k/tn0l6WfOk2BdtSrsCAwEAAaOCBAowggQGMB0GA1UdDgQWBBSv\n"
"CIbURu1HyBv/G+qr2PLWhO7FhjAfBgNVHSMEGDAWgBSYzJLQRjA2jLDtmA1yUalH\n"
"TN2+ITCCATIGA1UdHwSCASkwggElMIIBIaCCAR2gggEZhkdodHRwOi8vY2EuY2Vy\n"
"bi5jaC9jYS9jcmwvQ0VSTiUyMFRydXN0ZWQlMjBDZXJ0aWZpY2F0aW9uJTIwQXV0\n"
"aG9yaXR5LmNybIaBzWxkYXA6Ly8vQ049Q0VSTiUyMFRydXN0ZWQlMjBDZXJ0aWZp\n"
"Y2F0aW9uJTIwQXV0aG9yaXR5LENOPUNFUk5QS0ksQ049Q0RQLENOPVB1YmxpYyUy\n"
"MEtleSUyMFNlcnZpY2VzLENOPVNlcnZpY2VzLENOPUNvbmZpZ3VyYXRpb24sREM9\n"
"Y2VybixEQz1jaD9jZXJ0aWZpY2F0ZVJldm9jYXRpb25MaXN0P2Jhc2U/b2JqZWN0\n"
"Q2xhc3M9Y1JMRGlzdHJpYnV0aW9uUG9pbnQwggEvBggrBgEFBQcBAQSCASEwggEd\n"
"MIHFBggrBgEFBQcwAoaBuGxkYXA6Ly8vQ049Q0VSTiUyMFRydXN0ZWQlMjBDZXJ0\n"
"aWZpY2F0aW9uJTIwQXV0aG9yaXR5LENOPUFJQSxDTj1QdWJsaWMlMjBLZXklMjBT\n"
"ZXJ2aWNlcyxDTj1TZXJ2aWNlcyxDTj1Db25maWd1cmF0aW9uLERDPWNlcm4sREM9\n"
"Y2g/Y0FDZXJ0aWZpY2F0ZT9iYXNlP29iamVjdENsYXNzPWNlcnRpZmljYXRpb25B\n"
"dXRob3JpdHkwUwYIKwYBBQUHMAKGR2h0dHA6Ly9jYS5jZXJuLmNoL2NhL2NybC9D\n"
"RVJOJTIwVHJ1c3RlZCUyMENlcnRpZmljYXRpb24lMjBBdXRob3JpdHkuY3J0MA4G\n"
"A1UdDwEB/wQEAwIFoDA9BgkrBgEEAYI3FQcEMDAuBiYrBgEEAYI3FQiDvdAJgu2N\n"
"DYbtiyuB3vU3hYDQYh6Fv7oDhMTMTAIBZAIBCTApBgNVHSUEIjAgBggrBgEFBQcD\n"
"AgYIKwYBBQUHAwQGCisGAQQBgjcKAwQwFwYDVR0gBBAwDjAMBgorBgEEAWAKAgEB\n"
"MDUGCSsGAQQBgjcVCgQoMCYwCgYIKwYBBQUHAwIwCgYIKwYBBQUHAwQwDAYKKwYB\n"
"BAGCNwoDBDBLBgNVHREERDBCoCcGCisGAQQBgjcUAgOgGQwXYWRyaWVuLmRldnJl\n"
"c3NlQGNlcm4uY2iBF2Fkcmllbi5kZXZyZXNzZUBjZXJuLmNoMEQGCSqGSIb3DQEJ\n"
"DwQ3MDUwDgYIKoZIhvcNAwICAgCAMA4GCCqGSIb3DQMEAgIAgDAHBgUrDgMCBzAK\n"
"BggqhkiG9w0DBzANBgkqhkiG9w0BAQUFAAOCAQEAWUeMl2gTMQYRgdeh1M2XlhVp\n"
"XxVIiTAuWE3Js1g2OIag+unUWAVnsrDW4xZzy0aXd27po5nN8nfedHhpolsiXYSM\n"
"VPwVykXGtf2H+X9x/f4jsNemCTBcU2mSUJANYVUBZxxqr+HBCEVdUV/c11m6gcEe\n"
"zmfukbJgb6lCXZuVZdCFViySBt/eX40ObqexREtSCArRafP5jwda9LinzqFOkw5f\n"
"2UzqCBHSpl3a2RN7jLhbKRIiodbfKJK63+y2VU5/Y72jPreAFF+uTuZ0ZY1ziF6W\n"
"jUquwQ2TKhf+R/hN7x+Yiw3yH2xfl8RcVxy8ejYWaHYw6ZTGcs6rwCHnOVZOuA==\n"
"-----END CERTIFICATE-----\n";



void test_credential(){
    char buffer[6000]= {0};

    try{
        ssize_t ret = convert_x509_to_p12(private_key, cert_key, NULL, buffer, 6000);
        assert_true_with_message(ret >0, " convert credential failure ");
        FILE * f= fopen("/tmp/test_cert.p12","w+");
        fwrite(buffer, sizeof(char), ret, f);
        fclose(f);
        printf("p12 credential : %s",buffer);
    }catch(Glib::Error & e){
        assert_true_with_message(FALSE, " Glib::Error throw %s %d %s", g_quark_to_string(e.domain()), e.code(), e.what().c_str());

    }catch(std::exception & e){
        assert_true_with_message(FALSE, " std::exception throw %s", e.what());
    }

}


void test_false_cert(){
    char buffer[6000]= {0};

    try{
        ssize_t ret = convert_x509_to_p12(private_key, false_cert_key, NULL, buffer, 6000);
        assert_true_with_message(FALSE, " Must fail, incorrect cert");
    }catch(Glib::Error & e){
        assert_true_with_message( e.code()== UTILS_SECURITY_ERROR_CERT, " Glib::Error throw %s %d %s", g_quark_to_string(e.domain()), e.code(), e.what().c_str());

    }catch(std::exception & e){
        assert_true_with_message(FALSE, " std::exception throw %s", e.what());
    }

}

void proxy_cert_test(){
    char buffer[6000]= {0};

    try{
        ssize_t ret = convert_x509_to_p12(proxy_cert, proxy_cert, NULL, buffer, 6000);
        assert_true_with_message(ret >0, " convert credential failure ");
        FILE * f= fopen("/tmp/test_cert.p12","w+");
        fwrite(buffer, sizeof(char), ret, f);
        fclose(f);
        printf("p12 credential : %s",buffer);
    }catch(Glib::Error & e){
        assert_true_with_message(FALSE, " Glib::Error throw %s %d %s", g_quark_to_string(e.domain()), e.code(), e.what().c_str());

    }catch(std::exception & e){
        assert_true_with_message(FALSE, " std::exception throw %s", e.what());
    }

}

TestSuite * credential_suite (void)
{
        TestSuite *s2 = create_test_suite();
        // verbose test case /
        add_test(s2, test_credential);
        add_test(s2, test_false_cert);
        add_test(s2, proxy_cert_test);

        return s2;
 }

