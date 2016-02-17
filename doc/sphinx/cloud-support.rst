Cloud support
=============

Currently, there is support for the Amazon S3 and Microsoft Azure cloud services.

Amazon S3
---------

Amazon S3 has two signature calculation algorithms, called v2 and v4. The first one
is being deprecated, and new Amazon data-centers only support the second one.

Using v2 requires that you know an access key and its associated secret key. Your
secret key is never transmitted to the network; instead, it's used to calculate a
signature to sign each request.

The v4 algorithm requires an extra parameter, which is the region your bucket is located
in. If you specify a region, davix will use v4 to authenticate your requests, otherwise, v2.

An S3 URL can take two forms: ::

    https://bucket-name.example.org/dir/file
    https://s3-region.example.org/bucket-name/dir/file

The authentication algorithm needs to know which one of the two forms you are using, otherwise
you'll encounter errors. Davix assumes by default that S3 URLs have the first form; specify
``s3alternate`` to use the second. ::

  $ davix-get --s3accesskey xxx --s3secretkey yyy --s3region zzz --s3alternate https://s3-region.example.org/bucket-name/dir/file

Microsoft Azure
---------------

We currently support Microsoft Azure blob storage. It's very similar to S3 and acts as a key-value store,
letting you retrieve objects (blobs) by specifying their path. All you need is an azure key: ::

  $ davix-ls --azurekey xxx https://your-username.blob.core.windows.net/example-bucket/ 
