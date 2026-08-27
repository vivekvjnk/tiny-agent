#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <cstdint>

struct BusRange {
    uint64_t child_base;
    uint64_t parent_base;
    uint64_t size;
};

struct BusNode {
    std::string name;
    std::vector<BusRange> ranges;
    BusNode* parent = nullptr;
    std::vector<BusNode*> children;
};

std::optional<uint64_t> translate_address(const BusNode* leaf_node, uint64_t child_bus_addr) {
    const BusNode* current = leaf_node;
    uint64_t current_addr = child_bus_addr;

    while (current != nullptr) {
        // If ranges is empty, perform 1:1 identity translation
        if (current->ranges.empty()) {
            current = current->parent;
            continue;
        }

        bool match_found = false;

        for (const auto& range : current->ranges) {
            // Check if current_addr falls within [child_base, child_base + size)
            if (current_addr >= range.child_base && current_addr < range.child_base + range.size) {
                current_addr = range.parent_base + (current_addr - range.child_base);
                match_found = true;
                break;
            }
        }

        // Address is out of bounds for all declared range windows at this node level
        if (!match_found) {
            return std::nullopt;
        }

        current = current->parent;
    }

    return current_addr;
}

