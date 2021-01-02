#include "NodeEntry.h"
#include "DirectoryEntry.h"
#include "../utils/utils.h"

DirectoryEntry * NodeEntry::getParent() {
    if (this->parent) {
        return dynamic_cast<DirectoryEntry *>(this->parent);
    }
    return nullptr;
}

std::string NodeEntry::getPath() {
    if (parent) {
        return parent->getPath() + parent->getName() + OS_PATH_SEPARATOR;
    }
    return OS_PATH_SEPARATOR;
}

std::string NodeEntry::getFullPath() {
    return getPath() + getName();
}

void NodeEntry::setParent(DirectoryEntry *_parent) {
    this->parent = dynamic_cast<NodeEntry *>(_parent);
}
