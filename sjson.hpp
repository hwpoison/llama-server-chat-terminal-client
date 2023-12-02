#ifndef __SJSON_H__
#define __SJSON_H__
#include <iostream>
#include <vector>
#include <string>
#include "yyjson/src/yyjson.h"

// basic wrapper for yyjson
class sjson {
    public:
        sjson(const char* filename); // Read from file
        sjson(yyjson_doc* json_doc); // Read from raw json using 'json::open(json::read(rawstr))''

        yyjson_doc*         open(const char* filename);
        static yyjson_doc*  read(const char* raw_json);
        bool                is_opened();
        
        yyjson_val*         get_value(const char *keys[]);
        const char*         get_str(const char *keys[]);
        int64_t             get_int(const char *keys[]);
        bool                get_bool(const char *keys[]);
        double              get_real(const char *keys[]);
        
        yyjson_doc*         get_current_doc();
        yyjson_val*         get_current_root();
        const char*         get_type(const char *keys[]);
        
        template <typename T> void set_to_member(yyjson_val *doc, const char* key_name, T& structure, char* T::*member);
        template <typename T> void set_to_member(yyjson_val *doc, const char* key_name, T& structure, int T::*member);
        template <typename T> void set_to_member(yyjson_val *doc, const char* key_name, T& structure, float T::*member);
        template <typename T> void set_to_member(yyjson_val *doc, const char* key_name, T& structure, std::vector<std::string> T::*member);
        template <typename T> void set_to_member(yyjson_val *doc, const char* key_name, T& structure, bool T::*member);
        template <typename T> void set_to_member(yyjson_val *doc, const char* key_name, T& structure, std::string T::*member);
        
        ~sjson();

    private:
        std::string doc_filename;
        yyjson_doc *current_doc = NULL;
        yyjson_val *current_doc_root = NULL;
        
        // Read JSON file, allowing comments and trailing commas
        yyjson_read_flag flg = YYJSON_READ_ALLOW_COMMENTS | YYJSON_READ_ALLOW_TRAILING_COMMAS;
        yyjson_read_err err;
};

// set value to struct member
template <typename T>
void sjson::set_to_member(yyjson_val *doc, const char* key_name, T& structure, char* T::*member) {
    yyjson_val *value = yyjson_obj_get(doc, key_name);
    if(yyjson_is_str(value))
        structure.*member = (char*)yyjson_get_str(value);
}

template <typename T>
void sjson::set_to_member(yyjson_val *doc, const char* key_name, T& structure, int T::*member) {
    yyjson_val *value = yyjson_obj_get(doc, key_name);
    if(yyjson_is_int(value))
        structure.*member = yyjson_get_int(value);
}

template <typename T>
void sjson::set_to_member(yyjson_val *doc, const char* key_name, T& structure, float T::*member) {
    yyjson_val *value = yyjson_obj_get(doc, key_name);
    if(yyjson_is_real(value))
        structure.*member = yyjson_get_real(value);
}

template <typename T>
void sjson::set_to_member(yyjson_val *doc, const char* key_name, T& structure, std::vector<std::string> T::*member) {
    yyjson_val *value = yyjson_obj_get(doc, key_name);
    size_t idx, max;
    yyjson_val *hit;
    yyjson_arr_foreach(value, idx, max, hit)
        (structure.*member).push_back(yyjson_get_str(hit));
}

template <typename T>
void sjson::set_to_member(yyjson_val *doc, const char* key_name, T& structure, bool T::*member) {
    yyjson_val *value = yyjson_obj_get(doc, key_name);
    if(yyjson_is_bool(value))
        structure.*member = yyjson_get_bool(value);
}

template <typename T>
void sjson::set_to_member(yyjson_val *doc, const char* key_name, T& structure, std::string T::*member) {
    yyjson_val *value = yyjson_obj_get(doc, key_name);
    if(yyjson_is_str(value))
        structure.*member = yyjson_get_str(value);
}
#endif