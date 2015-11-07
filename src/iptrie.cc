/*
 * Copyright (c) 2011, OmniTI Computer Consulting, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name OmniTI Computer Consulting, Inc. nor the names
 *       of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written
 *       permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "btrie.h"

#include <v8.h>
#include <node.h>
#include <node_object_wrap.h>
#include <assert.h>

using namespace v8;
using namespace node;

class IPTrie : public ObjectWrap {
  public:
    static Persistent<FunctionTemplate> s_ct;
    static void
    Initialize(v8::Handle<v8::Object> target) {
      Isolate *isolate = target->GetIsolate();
      HandleScope scope(isolate);

      Local<FunctionTemplate> t = FunctionTemplate::New(isolate, New);
      s_ct.Reset(isolate, t);
      t->InstanceTemplate()->SetInternalFieldCount(3);
      t->SetClassName(String::NewFromUtf8(isolate, "IPTrie", String::kInternalizedString));

      NODE_SET_PROTOTYPE_METHOD(t, "add", Add);
      NODE_SET_PROTOTYPE_METHOD(t, "del", Del);
      NODE_SET_PROTOTYPE_METHOD(t, "find", Find);


      target->Set(String::NewFromUtf8(isolate, "IPTrie", String::kInternalizedString),
                  t->GetFunction());
    }

    struct obj_baton_t {
      IPTrie *iptrie;
      Persistent<Value> val;
    };

    static void delete_baton(void *vb) {
      obj_baton_t *b = (obj_baton_t *)vb;
      if(!b) return;
      b->val.Reset();
      delete b;
    }

    IPTrie() : tree4(NULL), tree6(NULL) { }
    ~IPTrie() {
      drop_tree(&tree4, delete_baton);
      drop_tree(&tree6, delete_baton);
    }

    int Add(const char *ip, int prefix, Handle<Value> dv) {
      int family, rv;
      union {
        struct in_addr addr4;
        struct in6_addr addr6;
      } a;

      family = AF_INET;
      rv = inet_pton(family, ip, &a);
      if(rv != 1) {
        family = AF_INET6;
        rv = inet_pton(family, ip, &a);
        if(rv != 1) {
          return 0;
        }
      }
      obj_baton_t *baton = new obj_baton_t();
      baton->iptrie = this;
      baton->val.Reset(Isolate::GetCurrent(), dv);

      if(family==AF_INET) add_route_ipv4(&tree4, &a.addr4, prefix, baton);
      else add_route_ipv6(&tree6, &a.addr6, prefix, baton);
      return 1;
    }

    int Del(const char *ip, int prefix) {
      int family, rv;
      union {
        struct in_addr addr4;
        struct in6_addr addr6;
      } a;

      family = AF_INET;
      rv = inet_pton(family, ip, &a);
      if(rv != 1) {
        family = AF_INET6;
        rv = inet_pton(family, ip, &a);
        if(rv != 1) {
          return 0;
        }
      }
      if(family==AF_INET) return del_route_ipv4(&tree4, &a.addr4, prefix, delete_baton);
      else return del_route_ipv6(&tree6, &a.addr6, prefix, delete_baton);
    }

    obj_baton_t *Find(const char *query) {
      int family, rv;
      unsigned char pl;
      union {
        struct in_addr addr4;
        struct in6_addr addr6;
      } a;

      family = AF_INET;
      rv = inet_pton(family, query, &a);
      if(rv != 1) {
        family = AF_INET6;
        rv = inet_pton(family, query, &a);
        if(rv != 1) {
          return NULL;
        }
      }
      if(family==AF_INET) return (obj_baton_t *)find_bpm_route_ipv4(&tree4, &a.addr4, &pl);
      else return (obj_baton_t *)find_bpm_route_ipv6(&tree6, &a.addr6, &pl);
    }

  protected:
    static void New(const FunctionCallbackInfo<Value> &args) {
      IPTrie *iptrie = new IPTrie();
      iptrie->Wrap(args.This());

      args.GetReturnValue().Set(args.This());
    }

    static void Add(const FunctionCallbackInfo<Value> &args) {
      Isolate *isolate = args.GetIsolate();
      HandleScope scope(isolate);

      if (args.Length() < 1 || !args[0]->IsString()) {
        isolate->ThrowException(
                Exception::TypeError(
                    String::NewFromUtf8(isolate, "First argument must be an IP.")));
        return;
      }
      if (args.Length() < 2 || !args[1]->IsNumber()){
        isolate->ThrowException(
          Exception::TypeError(
            String::NewFromUtf8(isolate, "Second argument must be a prefix length")));
        return;
      }
      if (args.Length() < 3) {
        isolate->ThrowException(
          Exception::TypeError(
            String::NewFromUtf8(isolate, "Third argument must exist")));
        return;
      }

      String::Utf8Value ipaddress(args[0]->ToString());
      int prefix_len = args[1]->ToUint32()->Value();

      IPTrie *iptrie = ObjectWrap::Unwrap<IPTrie>(args.This());
      Handle<Value> data = args[2];
      if(iptrie->Add(*ipaddress, prefix_len, data) == 0) {
        isolate->ThrowException(
          Exception::TypeError(
            String::NewFromUtf8(isolate, "Could not parse IP")));
        return;
      }
    }

    static void Del(const FunctionCallbackInfo<Value> &args) {
      Isolate *isolate = args.GetIsolate();

      if (args.Length() < 1 || !args[0]->IsString()) {
        isolate->ThrowException(
                Exception::TypeError(
                    String::NewFromUtf8(isolate, "First argument must be an IP.")));
        return;
      }
      if (args.Length() < 2 || !args[1]->IsNumber()){
        isolate->ThrowException(
          Exception::TypeError(
            String::NewFromUtf8(isolate, "Second argument must be a prefix length")));
        return;
      }

      String::Utf8Value ipaddress(args[0]->ToString());
      int prefix_len = args[1]->ToUint32()->Value();

      IPTrie *iptrie = ObjectWrap::Unwrap<IPTrie>(args.This());
      int success = iptrie->Del(*ipaddress, prefix_len);

      args.GetReturnValue().Set(success ? True(isolate) : False(isolate));
    }

    static void Find(const FunctionCallbackInfo<Value> &args) {
      Isolate *isolate = args.GetIsolate();

      if (args.Length() < 1 || !args[0]->IsString()) {
        isolate->ThrowException(
                Exception::TypeError(
                    String::NewFromUtf8(isolate, "Required argument: ip address.")));
        return;
      }

      String::Utf8Value ipaddress(args[0]->ToString());

      IPTrie *iptrie = ObjectWrap::Unwrap<IPTrie>(args.This());
      obj_baton_t *d = iptrie->Find(*ipaddress);
      if(d != NULL) {
        args.GetReturnValue().Set(d->val);
      }
    }

  private:
    btrie tree4;
    btrie tree6;
};

Persistent<FunctionTemplate> IPTrie::s_ct;

extern "C" {
static void init(Handle<Object> target) {
  IPTrie::Initialize(target);
}
NODE_MODULE(iptrie, init);
}
