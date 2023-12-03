//
//  base64.h
//
//  Created by HongyongXiao on 2021/11/20.
//

#pragma once

#ifndef base64_hpp
#define base64_hpp

string base64Encode(const uint8_t *in, size_t len);
bool base64Decode(const char *in, size_t len, string &out);
inline string base64Decode(const char *in, size_t len) {
    string out;
    base64Decode(in, len, out);
    return out;
}

#endif /* base64_hpp */
