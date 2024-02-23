#include "sjson.hpp"

sjson::sjson(yyjson_doc* json_doc){
    current_doc = json_doc;
    current_doc_root = yyjson_doc_get_root(current_doc);
}

sjson::sjson(const char* filename){
    doc_filename = filename;
    open(filename);
}

yyjson_doc* sjson::read(const char* raw_json){
    return yyjson_read(raw_json, strlen(raw_json), 0);
}

yyjson_doc* sjson::open(const char* filename) {
    current_doc = yyjson_read_file(filename, flg, NULL, &err);
    if (!current_doc){
        std::cout << "Problem reading the file \"" << filename << "\": read error (" << err.code << "): " << err.msg << " at position:" << err.pos << std::endl;
        return NULL;
    }
    
    current_doc_root = yyjson_doc_get_root(current_doc);
    return current_doc;
};

yyjson_val* sjson::get_value(const char *keys[]) {
    yyjson_val *tmp = NULL;
    for (int i = 0; *keys[i] != '\0'; keys++) {
        if (tmp == NULL)
            tmp = yyjson_obj_get(current_doc_root, keys[i]);
        else
            tmp = yyjson_obj_get(tmp, keys[i]);
        if (tmp == NULL) {
            std::cerr << "Error getting the value \"" << keys[i] << "\" from \"" << doc_filename << "\"" << std::endl;
            return NULL;
        }
    }
    return tmp;
};

const char* sjson::get_str(const char *keys[]){
    return yyjson_get_str(get_value(keys));
}

int64_t sjson::get_int(const char *keys[]){
    return yyjson_get_sint(get_value(keys));
}

bool sjson::is_opened(){
    return current_doc==NULL?false:true;
}

bool sjson::get_bool(const char *keys[]){
    return yyjson_get_bool(get_value(keys));
}

double sjson::get_real(const char *keys[]){
    return yyjson_get_real(get_value(keys));
}

yyjson_doc* sjson::get_current_doc(){
    return current_doc;
}

yyjson_val* sjson::get_current_root(){
    return current_doc_root;
}

const char * sjson::get_type(const char *keys[]) {
    yyjson_val *value = get_value(keys);
    if (value) {
        return yyjson_get_type_desc(value);
    } else {
        std::cerr << "Failed to get type" << std::endl;
        return NULL;
    }
};


sjson::~sjson() { yyjson_doc_free(current_doc);}