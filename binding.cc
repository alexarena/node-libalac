// ALAC.node, © 2014 Stéphan Kochen
// MIT-licensed. (See the included LICENSE file.)

#include <v8.h>
#include <node.h>
#include <nan.h>
#include <node_buffer.h>
#include <ALACBitUtilities.h>
#include <ALACDecoder.h>

using namespace v8;
using namespace node;

namespace alac {


static Nan::Persistent<String> frames_per_packet_symbol;
static Nan::Persistent<String> sample_rate_symbol;
static Nan::Persistent<String> channels_symbol;
static Nan::Persistent<String> bit_depth_symbol;
static Nan::Persistent<String> cookie_symbol;

// XXX: Stolen from convert-utility.
enum
{
    kTestFormatFlag_16BitSourceData    = 1,
    kTestFormatFlag_20BitSourceData    = 2,
    kTestFormatFlag_24BitSourceData    = 3,
    kTestFormatFlag_32BitSourceData    = 4
};

void
throw_alac_error(int32_t ret)
{
  // XXX: The remaining values are unused in the codec itself.
  switch (ret) {
    case kALAC_ParamError:
      Nan::ThrowError("ALAC error: invalid parameter");
    case kALAC_MemFullError:
      Nan::ThrowError("ALAC error: out of memory");
    default:
      Nan::ThrowError("ALAC error: unknown error");
  }
}

class Decoder : public Nan::ObjectWrap
{
public:
  static void
  Initialize(Local<Object> target)
  {
    Local<FunctionTemplate> t = Nan::New<FunctionTemplate>(New);
    t->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(t, "decode", Decode);

    //problem #1
    // Nan::Set(target, Nan::New<String>("Decoder").ToLocalChecked(), t->GetFunction());
  }

  virtual
  ~Decoder() {}

private:
  Decoder() : dec_() {}

  static void
  New(const Nan::FunctionCallbackInfo<v8::Value>& info)
  {
    Nan::HandleScope scope;

    Local<Object> o = Local<Object>::Cast(info[0]);
    Local<Value> v;

    Decoder *d = new Decoder();

    // Fill parameters.
    v = o->Get(Nan::New<String>(cookie_symbol));
    //problem #2
    // d->channels_= o->Get(Nan::New<String>(channels_symbol))->Uint32Value();
    d->channels_ = 2;

    //problem #3
    // Ed->frames_ = o->Get(Nan::New<String>(frames_per_packet_symbol))->Uint32Value();

    // Init decoder.
    int32_t ret = d->dec_.Init(Buffer::Data(v), Buffer::Length(v));
    if (ret != ALAC_noErr) {
      delete d;
      throw_alac_error(ret);
      return;
    }

    // Init self.
    d->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  }

  static void
  Decode(const Nan::FunctionCallbackInfo<v8::Value>& info)
  {
    Nan::HandleScope scope;

    Decoder *d = Nan::ObjectWrap::Unwrap<Decoder>(info.This());

    BitBuffer in;
    BitBufferInit(&in, (uint8_t *) Buffer::Data(info[0]), Buffer::Length(info[0]));
    uint8_t *out = (uint8_t *) Buffer::Data(info[1]);

    uint32_t numFrames;
    int32_t ret = d->dec_.Decode(&in, out, d->frames_, d->channels_, &numFrames);
    if (ret != ALAC_noErr) {
      throw_alac_error(ret);
      return;
    }

    info.GetReturnValue().Set(Nan::New<Uint32>(numFrames));
  }

private:
  ALACDecoder dec_;
  uint32_t channels_;
  uint32_t frames_;
};


static void
Initialize(Local<Object> target)
{
  Nan::HandleScope scope;

  frames_per_packet_symbol.Reset(Nan::New<String>("framesPerPacket").ToLocalChecked());
  sample_rate_symbol.Reset(Nan::New<String>("sampleRate").ToLocalChecked());
  channels_symbol.Reset(Nan::New<String>("channels").ToLocalChecked());
  bit_depth_symbol.Reset(Nan::New<String>("bitDepth").ToLocalChecked());
  cookie_symbol.Reset(Nan::New<String>("cookie").ToLocalChecked());

  NODE_DEFINE_CONSTANT(target, kALACDefaultFramesPerPacket);
  NODE_DEFINE_CONSTANT(target, kALACMaxEscapeHeaderBytes);

  Decoder::Initialize(target);
}


} // namespace alac

extern "C" void
init(Local<Object> target)
{
  alac::Initialize(target);
}

NODE_MODULE(binding, init)
