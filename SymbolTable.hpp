#ifndef SYMBOL_TABLE_H  
#define SYMBOL_TABLE_H  

#include <unordered_map>  
#include <string>  
#include "ST.hpp"  

class SymbolTable {
public:  
   SymbolTable();  

   void insert(const std::string& symbol, const ST& entry);  
   ST fetch(const std::string& symbol) const;  
   size_t size() const;  

private:  
   std::unordered_map<std::string, ST> table;  
};  

#endif