#include "SymbolTable.hpp"
#include "ST.hpp"

SymbolTable::SymbolTable() {}

void SymbolTable::insert(const std::string& symbol, const ST& entry) {
    table[symbol] = entry;
}

ST SymbolTable::fetch(const std::string& symbol) const {
    auto it = table.find(symbol);
    if (it != table.end()) {
        return it->second;
    }
    return ST{ ST::STRING, "" };
}

size_t SymbolTable::size() const {
	return table.size();
}