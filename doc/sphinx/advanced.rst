Advanced functionalities
========================

This page documents some specific behaviors of davix not found in other HTTP clients.

Multi-range queries
-------------------

During a multi-range query, the client requests multiple contiguous chunks of a file. Not all
servers support this, however. Some will understand the request and return a proper multi-chunk
response, some will return the entire file, some will only give you the first range, and some
will return an error.

Davix will make an effort to work correctly in all of the above cases.

* If the server gives back a correctly-formed multi-part response, no need to do anything extra.
* If the server gives back the whole file (200 OK response), davix can do two things:

  * abort the transfer by shutting down the TCP connection, and try a :ref:`multisimulation`.
  * download the entire file and extract from that the desired ranges.

  The decision will depend on the file size. It would not make sense to download a
  2 GB file to extract only a few bytes, so, if the file is too large, davix will take the first option.
* If the server gives back the first range or gives an error, davix will do a :ref:`multisimulation`.

If you know that a server does not support multi-range, you can instruct davix to jump straight to simulation
by using a URL fragment parameter. ::

  https://example.org/somefile#multirange=false


.. _multisimulation:

Simulation of a multi-range query
---------------------------------

A multi-range query can be simulated by using a series of consecutive single-range queries,
one for each desired range. The obvious drawback is performance, since you now have to issue
many more requests.

Davix will try to lessen this performance impact by using multiple parallel HTTP connections.
The default number is 10 connections, but you can override it with a URL fragment parameter. ::

  https://example.org/somefile#multirange=false&nconnections=30


