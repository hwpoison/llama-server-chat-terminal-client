#include <iostream>
#include <vector>
#include <string>
#include "yyjson.h"

// basic wrapper for yyjson
namespace simplejson {

    class json {
        public:
            json(const char* filename); // Read from file
            json(yyjson_doc* json_doc); // Read from raw json using 'json::open(json::read(str))''

            yyjson_doc*         open(const char* filename);
            static yyjson_doc*  read(const char* raw_json);
            
            yyjson_val*         get_value(const char *keys[]);
            const char*         get_str(const char *keys[]);
            int64_t             get_int(const char *keys[]);
            bool                get_bool(const char *keys[]);
            double              get_real(const char *keys[]);
            bool                is_opened();

            const char*         get_type(const char *keys[]);

            template <typename T>
            void setMember(yyjson_val *doc, const char* key_name, T& structure, char* T::*member) {
                yyjson_val *value = yyjson_obj_get(doc, key_name);
                if(yyjson_is_str(value))
                    structure.*member = (char*)yyjson_get_str(value);
            }

            template <typename T>
            void setMember(yyjson_val *doc, const char* key_name, T& structure, int T::*member) {
                yyjson_val *value = yyjson_obj_get(doc, key_name);
                if(yyjson_is_int(value))
                    structure.*member = yyjson_get_int(value);
            }

            template <typename T>
            void setMember(yyjson_val *doc, const char* key_name, T& structure, float T::*member) {
                yyjson_val *value = yyjson_obj_get(doc, key_name);
                if(yyjson_is_real(value))
                    structure.*member = yyjson_get_real(value);
            }

            template <typename T>
            void setMember(yyjson_val *doc, const char* key_name, T& structure, std::vector<std::string> T::*member) {
                yyjson_val *value = yyjson_obj_get(doc, key_name);
                size_t idx, max;
                yyjson_val *hit;
                yyjson_arr_foreach(value, idx, max, hit)
                    (structure.*member).push_back(yyjson_get_str(hit));
            }


            template <typename T>
            void setMember(yyjson_val *doc, const char* key_name, T& structure, bool T::*member) {
                yyjson_val *value = yyjson_obj_get(doc, key_name);
                if(yyjson_is_bool(value))
                    structure.*member = yyjson_get_bool(value);
            }
            ~json() { yyjson_doc_free(current_doc);}

            yyjson_doc* getCurrentDoc(){
                return current_doc;
            }

        private:
            std::string doc_filename;
            yyjson_doc *current_doc = NULL;
            yyjson_val *current_doc_root = NULL;
            
            // Read JSON file, allowing comments and trailing commas
            yyjson_read_flag flg = YYJSON_READ_ALLOW_COMMENTS | YYJSON_READ_ALLOW_TRAILING_COMMAS;
            yyjson_read_err err;
    };

    yyjson_doc* json::read(const char* raw_json){
        return yyjson_read(raw_json, strlen(raw_json), 0);
    }

    json::json(yyjson_doc* json_doc){
        current_doc = json_doc;
        current_doc_root = yyjson_doc_get_root(current_doc);
    }

    json::json(const char* filename){
        doc_filename = filename;
        open(filename);
    }

    yyjson_doc* json::open(const char* filename) {
        yyjson_doc *doc = yyjson_read_file(filename, flg, NULL, &err);
        if (!doc) {
            std::cout << "read error (" << err.code << "): " << err.msg << " at position:" << err.pos << std::endl;
            return NULL;
        }
        current_doc = doc;
        current_doc_root = yyjson_doc_get_root(current_doc);
        return doc;
    };

    yyjson_val* json::get_value(const char *keys[]) {
        yyjson_val *tmp = NULL;
        for (int i = 0; keys[i] != "\0"; keys++) {
            if (tmp == NULL) {
                tmp = yyjson_obj_get(current_doc_root, keys[i]);
            } else {
                tmp = yyjson_obj_get(tmp, keys[i]);
            }
            if (tmp == NULL) {
                std::cerr << "Error getting the value of \"" << keys[i] << "\" from \"" << doc_filename << "\"" << std::endl;
                return NULL;
            }
        }
        return tmp;
    };

    const char* json::get_str(const char *keys[]){
        return yyjson_get_str(get_value(keys));
    }

    int64_t json::get_int(const char *keys[]){
        return yyjson_get_sint(get_value(keys));
    }

    bool json::is_opened(){
        return current_doc!=NULL?true:false;
    }

    bool json::get_bool(const char *keys[]){
        return yyjson_get_bool(get_value(keys));
    }

    double json::get_real(const char *keys[]){
        return yyjson_get_real(get_value(keys));
    }

    const char * json::get_type(const char *keys[]) {
        yyjson_val *value = get_value(keys);
        if (value) {
            return yyjson_get_type_desc(value);
        } else {
            std::cerr << "Failed to get type" << std::endl;
            return NULL;
        }
    };
};
