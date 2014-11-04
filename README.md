## libalac [![Build Status](https://secure.travis-ci.org/stephank/node-libalac.png)](http://travis-ci.org/stephank/node-libalac)

Bindings to the official Apple Lossless (ALAC) encoder and decoder.

### Installing

    $ npm install libalac

### Encoder usage

The encoder is a regular stream:

    var alac = require('libalac');
    var enc = alac.encoder({
      sampleRate: 44100,
      channels: 2,
      bitDepth: 16
    });
    input.pipe(enc).pipe(output);

`alac.encoder()` must have an object argument, which can contain regular
readable and writable stream options, along with the following:

 - `sampleRate` (in Hz) *required*
 - `channels`, *required*
 - `bitDepth`, *rqeuired*
 - `framesPerPacket`, defaults to `4096` (usually no need to modify this)

The encoder object also has the following properties:

 - `cookie`, a buffer containing the ALAC magic cookie. These are parameters
   for the decoder, and is what you'd place in e.g. the `kuki` chunk of a
   CAF-file.

 - `packets`, array of sizes of packets in the stream. This array is only ever
   pushed to, and can be modified, or even replaced with an array-like object,
   as long as it has a `push` method.

 - `sampleRate`, `channels`, `bitDepth`, and `framesPerPacket` containing the
   final parameters used in the encoder.

Note that the encoder always reads input in native byte order!

### Decoder usage

The decoder is a regular stream:

    var alac = require('libalac');
    var dec = alac.decoder({
      cookie: cookie,
      channels: 2,
      bitDepth: 16,
      framesPerPacket: 4096,
      packets: packets
    });
    input.pipe(dec).pipe(output);

`alac.decoder()` must have an object argument, which can contain regular
readable and writable stream options, along with the following:

 - `cookie`, *required*, a buffer as generated by the encoder
 - `channels`, *required*
 - `bitDepth`, *required*
 - `framesPerPacket`, *required*
 - `packets`, an array of packet sizes

Instead of providing `packets` up front, it can also be provided (or extended)
during streaming by calling `packets(arr)` one or more times.

Usually, these parameters can all be extracted from the container format. For
example, in a CAF-file, they live in the `desc`, `kuki` and `pakt` chunks.

Note that the decoder always outputs in native byte order!

### Hacking the code

    git clone https://github.com/stephank/libalac.git
    cd libalac
    npm install
    npm test
