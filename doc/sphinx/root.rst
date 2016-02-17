Usage in ROOT
=============

We have also created a davix plugin for ROOT_, TDavixFile. It can be used to perform I/O on HTTP, WebDAV or S3.

.. _ROOT: https://root.cern.ch/

Installation from EPEL repositories
-----------------------------------

#. Install root and root-davix ::

    $ sudo yum install root root-net-davix

#. Enable the davix plugin by default ::

    $ sudo sed -i 's/Davix.UseOldClient: yes/Davix.UseOldClient: no/g' /usr/share/root/system.rootrc

#. Tune configuration of rootrc with your HTTP parameters. See the available options in :ref:`configureTDavix`. ::

    $ vim /usr/share/root/system.rootrc

Installation from source
------------------------

#. Download your favorite version of ROOT from https://root.cern.ch

#. Install the dependencies: cmake, openssl-devel, libxml2-devel

#. Run ``installDavix.sh`` which downloads and compiles davix.

#. Configure ROOT with ``--with-davix-incdir`` and ``--with-davix-libdir`` indicating the davix installation created by ``installDavix.sh``. Davix should be displayed in the list of "enabled modules" at the end of the configuration step.

#. Compile ROOT.

Usage
-----

Using Davix is completely transparent once installed and configured, just proceed like with any other TFile plugin. ::

    TFile* f  = TFile::Open("http://root.cern.ch/files/h1big.root")

Debugging
---------

* To use ROOT's logging, increase the gDebug variable level. ::

    $ gDebug = 2

* To use Davix's internal logging, use the Davix.Debug parameter. It can be used independently of gDebug. To display only Davix's logging infomation, set gDebug to 0 and Davix.Debug to 1-4, with 4 being most verbose. In code: ::

    $ gEnv->SetValue("Davix.Debug", 4);

* In system.rootrc: ::

    $ Davix.Debug: 4

.. _configureTDavix:

Configure TDavixFile
--------------------

TDavixFile can be configured via the **system.rootrc** file, via **ROOT environment variables** or via **TFile::Open flags** at runtime. All configuration parameters are documented inside system.rootrc. Main options:

+-------------------------------------------------------------+------------------------+----------------------+ 
| Feature                                                     | ROOT parameter         | Value                | 
+=============================================================+========================+======================+ 
| Enable/disable grid authentication support                  | Davix.GSI.GridMode     | y/n                  | 
+-------------------------------------------------------------+------------------------+----------------------+
| Enable/disable TLS certificate validity check               | Davix.GSI.CACheck      | y/n                  | 
+-------------------------------------------------------------+------------------------+----------------------+
| Specify S3 authentication tokens                            | Davix.S3.SecretKey     | string               | 
+                                                             +------------------------+----------------------+
|                                                             | Davix.S3.AccessKey     | string               | 
+-------------------------------------------------------------+------------------------+----------------------+
| Specify user x509 credentials in PEM format                 | Davix.GSI.UserCert     | filepath (string)    | 
+                                                             +------------------------+----------------------+
|                                                             | Davix.GSI.UserKey      | filepath (string)    | 
+-------------------------------------------------------------+------------------------+----------------------+
| Specify VOMS proxy to use                                   | Davix.GSI.UserProxy    | filepath (string)    | 
+-------------------------------------------------------------+------------------------+----------------------+
| Define log verbosity                                        | Davix.Debug            | integer (1-4)        | 
+-------------------------------------------------------------+------------------------+----------------------+

Running HTTP ROOT TDavixFile Benchmarks
---------------------------------------

Check this repository: https://github.com/cern-it-sdc-id/tdavixfile-bench-tools.git              

 


