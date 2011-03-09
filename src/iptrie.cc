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
#include <node_events.h>
#include <assert.h>

using namespace v8;
using namespace node;

class IPTrie : ObjectWrap {
  public:
    static Persistent<FunctionTemplate> s_ct;
    static void
    Initialize(v8::Handle<v8::Object> target) {
      HandleScope scope;

      Local<FunctionTemplate> t = FunctionTemplate::New(New);
      s_ct = Persistent<FunctionTemplate>::New(t);
      s_ct->InstanceTemplate()->SetInternalFieldCount(3);
      s_ct->SetClassName(String::NewSymbol("IPTrie"));

      NODE_SET_PROTOTYPE_METHOD(t, "add", Add);
      NODE_SET_PROTOTYPE_METHOD(t, "del", Del);
      NODE_SET_PROTOTYPE_METHOD(t, "find", Find);


      target->Set(String::NewSymbol("IPTrie"),
                  s_ct->GetFunction());
    }

    struct obj_baton_t {
      IPTrie *iptrie;
      Persistent<Value> val;
    };

    static void delete_baton(void *vb) {
      obj_baton_t *b = (obj_baton_t *)vb;
      if(!b) return;
      b->val.Dispose();
      delete b;
    }

    IPTrie() : tree4(NULL), tree6(NULL) { }
    ~IPTrie() {
      drop_tree(&tree4, delete_baton);
      drop_tree(&tree6, delete_baton);
    }

    int Add(const char *ip, int prefix, Handle<Value> dv) {
      HandleScope scope;
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
      baton->val = Persistent<Value>::New(dv);

      if(family==AF_INET) add_route_ipv4(&tree4, &a.addr4, prefix, baton);
      else add_route_ipv6(&tree6, &a.addr6, prefix, baton);
      return 1;
    }

    int Del(const char *ip, int prefix) {
      HandleScope scope;
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
      HandleScope scope;
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
    static Handle<Value> New(const Arguments& args) {
      HandleScope scope;

      IPTrie *iptrie = new IPTrie();
      iptrie->Wrap(args.This());

      return args.This();
    }

    static Handle<Value> Add(const Arguments &args) {
      HandleScope scope;

      if (args.Length() < 1 || !args[0]->IsString()) {
        return ThrowException(
                Exception::TypeError(
                    String::New("First argument must be an IP.")));
      }
      if (args.Length() < 2 || !args[1]->IsNumber()){
        return ThrowException(
          Exception::TypeError(
            String::New("Second argument must be a prefix length")));
      }
      if (args.Length() < 3) {
        return ThrowException(
          Exception::TypeError(
            String::New("Third argument must exist")));
      }

      String::Utf8Value ipaddress(args[0]->ToString());
      int prefix_len = args[1]->ToUint32()->Value();

      IPTrie *iptrie = ObjectWrap::Unwrap<IPTrie>(args.This());
      Handle<Value> data = args[2];
      if(iptrie->Add(*ipaddress, prefix_len, data) == 0) {
        return ThrowException(
          Exception::TypeError(
            String::New("Could not parse IP")));
      }

      return Undefined();
    }

    static Handle<Value> Del(const Arguments &args) {
      HandleScope scope;

      if (args.Length() < 1 || !args[0]->IsString()) {
        return ThrowException(
                Exception::TypeError(
                    String::New("First argument must be an IP.")));
      }
      if (args.Length() < 2 || !args[1]->IsNumber()){
        return ThrowException(
          Exception::TypeError(
            String::New("Second argument must be a prefix length")));
      }

      String::Utf8Value ipaddress(args[0]->ToString());
      int prefix_len = args[1]->ToUint32()->Value();

      IPTrie *iptrie = ObjectWrap::Unwrap<IPTrie>(args.This());
      int success = iptrie->Del(*ipaddress, prefix_len);

      return success ? True() : False();
    }

    static Handle<Value> Find(const Arguments &args) {
      HandleScope scope;

      if (args.Length() < 1 || !args[0]->IsString()) {
        return ThrowException(
                Exception::TypeError(
                    String::New("Required argument: ip address.")));
      }

      String::Utf8Value ipaddress(args[0]->ToString());

      IPTrie *iptrie = ObjectWrap::Unwrap<IPTrie>(args.This());
      obj_baton_t *d = iptrie->Find(*ipaddress);
      if(d == NULL) return Undefined();
      return d->val;
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
