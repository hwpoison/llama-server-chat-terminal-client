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


        ~sjson();

    private:
        std::string doc_filename;
        yyjson_doc *current_doc = NULL;
        yyjson_val *current_doc_root = NULL;
        
        // Read JSON file, allowing comments and trailing commas
        yyjson_read_flag flg = YYJSON_READ_ALLOW_COMMENTS | YYJSON_READ_ALLOW_TRAILING_COMMAS;
        yyjson_read_err err;
};
#endif