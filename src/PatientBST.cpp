#include "smarter/PatientBST.h"
#include <iomanip>

namespace smarter {

int PatientBST::compareKeys(const std::string& h1, const std::string& p1,
                            const std::string& h2, const std::string& p2) {
    if (h1 != h2) {
        return (h1 < h2) ? -1 : 1;
    }
    if (p1 != p2) {
        return (p1 < p2) ? -1 : 1;
    }
    return 0; // Both keys are identical
}

PatientBST::PatientBST(const PatientRegistry* registry)
    : root_(nullptr),
      size_(0),
      registry_(registry) {}

PatientBST::~PatientBST() {
    clear();
}

PatientBST::PatientBST(const PatientBST& other)
    : root_(cloneTree(other.root_)),
      size_(other.size_),
      registry_(other.registry_) {}

PatientBST& PatientBST::operator=(const PatientBST& other) {
    if (this != &other) {
        clear();
        root_ = cloneTree(other.root_);
        size_ = other.size_;
        registry_ = other.registry_;
    }
    return *this;
}

PatientBST::PatientBST(PatientBST&& other) noexcept
    : root_(other.root_),
      size_(other.size_),
      registry_(other.registry_) {
    other.root_ = nullptr;
    other.size_ = 0;
    other.registry_ = nullptr;
}

PatientBST& PatientBST::operator=(PatientBST&& other) noexcept {
    if (this != &other) {
        clear();
        root_ = other.root_;
        size_ = other.size_;
        registry_ = other.registry_;

        other.root_ = nullptr;
        other.size_ = 0;
        other.registry_ = nullptr;
    }
    return *this;
}

void PatientBST::setRegistry(const PatientRegistry* registry) {
    registry_ = registry;
}

const PatientRegistry* PatientBST::getRegistry() const {
    return registry_;
}

bool PatientBST::insertPatient(const std::string& hospitalId, const std::string& patientId) {
    bool success = false;
    root_ = insertHelper(root_, hospitalId, patientId, success);
    if (success) {
        ++size_;
    }
    return success;
}

BSTNode* PatientBST::insertHelper(BSTNode* node,
                                  const std::string& hospId,
                                  const std::string& patId,
                                  bool& success) {
    if (node == nullptr) {
        success = true;
        return new BSTNode(hospId, patId);
    }

    int cmp = compareKeys(hospId, patId, node->hospitalId, node->patientId);

    if (cmp < 0) {
        node->left = insertHelper(node->left, hospId, patId, success);
    } else if (cmp > 0) {
        node->right = insertHelper(node->right, hospId, patId, success);
    } else {
        // Duplicate composite key (hospitalId, patientId) rejected
        success = false;
    }

    return node;
}

bool PatientBST::searchPatient(const std::string& hospitalId, const std::string& patientId) const {
    return searchHelper(root_, hospitalId, patientId) != nullptr;
}

BSTNode* PatientBST::searchHelper(BSTNode* node,
                                  const std::string& hospId,
                                  const std::string& patId) const {
    if (node == nullptr) {
        return nullptr;
    }

    int cmp = compareKeys(hospId, patId, node->hospitalId, node->patientId);

    if (cmp < 0) {
        return searchHelper(node->left, hospId, patId);
    } else if (cmp > 0) {
        return searchHelper(node->right, hospId, patId);
    } else {
        return node;
    }
}

bool PatientBST::removePatient(const std::string& hospitalId, const std::string& patientId) {
    bool success = false;
    root_ = removeHelper(root_, hospitalId, patientId, success);
    if (success) {
        --size_;
    }
    return success;
}

BSTNode* PatientBST::removeHelper(BSTNode* node,
                                  const std::string& hospId,
                                  const std::string& patId,
                                  bool& success) {
    if (node == nullptr) {
        success = false;
        return nullptr;
    }

    int cmp = compareKeys(hospId, patId, node->hospitalId, node->patientId);

    if (cmp < 0) {
        node->left = removeHelper(node->left, hospId, patId, success);
    } else if (cmp > 0) {
        node->right = removeHelper(node->right, hospId, patId, success);
    } else {
        // Node found! Handle the 3 deletion cases:
        success = true;

        // Case 1 & Case 2: Node with 0 or 1 child
        if (node->left == nullptr) {
            BSTNode* temp = node->right;
            delete node;
            return temp;
        } else if (node->right == nullptr) {
            BSTNode* temp = node->left;
            delete node;
            return temp;
        }

        // Case 3: Node with 2 children
        // Replace with in-order successor (smallest node in right subtree)
        BSTNode* successor = findMin(node->right);
        node->hospitalId = successor->hospitalId;
        node->patientId = successor->patientId;

        // Recursively remove the successor from the right subtree
        bool subSuccess = false;
        node->right = removeHelper(node->right, successor->hospitalId, successor->patientId, subSuccess);
    }

    return node;
}

BSTNode* PatientBST::findMin(BSTNode* node) const {
    while (node != nullptr && node->left != nullptr) {
        node = node->left;
    }
    return node;
}

void PatientBST::displaySortedPatients(std::ostream& os) const {
    if (isEmpty()) {
        os << "PatientBST is currently empty.\n";
        return;
    }

    os << "--------------------------------------------------------------------------------\n";
    os << "  Sorted Patient Index (In-Order Traversal | " << size_ << " patients)\n";
    os << "--------------------------------------------------------------------------------\n";

    if (registry_ != nullptr) {
        os << std::left
           << std::setw(6)  << "No."
           << std::setw(12) << "Hosp ID"
           << std::setw(12) << "Patient ID"
           << std::setw(22) << "Name"
           << std::setw(6)  << "Age"
           << std::setw(8)  << "Gender"
           << std::setw(8)  << "Blood"
           << std::setw(14) << "Contact" << "\n";
    } else {
        os << std::left
           << std::setw(6)  << "No."
           << std::setw(14) << "Hospital ID"
           << std::setw(14) << "Patient ID" << "\n";
    }
    os << "--------------------------------------------------------------------------------\n";

    size_t rank = 1;
    inorderTraversal(root_, os, rank);
    os << "--------------------------------------------------------------------------------\n";
}

void PatientBST::inorderTraversal(BSTNode* node, std::ostream& os, size_t& rank) const {
    if (node == nullptr) {
        return;
    }

    // 1. Visit left subtree
    inorderTraversal(node->left, os, rank);

    // 2. Visit current node
    if (registry_ != nullptr) {
        const Patient* p = registry_->findPatient(node->hospitalId, node->patientId);
        if (p != nullptr) {
            os << std::left
               << std::setw(6)  << rank++
               << std::setw(12) << node->hospitalId
               << std::setw(12) << node->patientId
               << std::setw(22) << p->getName()
               << std::setw(6)  << p->getAge()
               << std::setw(8)  << p->getGender()
               << std::setw(8)  << p->getBloodGroup()
               << std::setw(14) << p->getContactPhone() << "\n";
        } else {
            os << std::left
               << std::setw(6)  << rank++
               << std::setw(12) << node->hospitalId
               << std::setw(12) << node->patientId
               << std::setw(22) << "(Demographics not in registry)" << "\n";
        }
    } else {
        os << std::left
           << std::setw(6)  << rank++
           << std::setw(14) << node->hospitalId
           << std::setw(14) << node->patientId << "\n";
    }

    // 3. Visit right subtree
    inorderTraversal(node->right, os, rank);
}

std::vector<std::pair<std::string, std::string>> PatientBST::getInorderKeys() const {
    std::vector<std::pair<std::string, std::string>> keys;
    collectInorderKeys(root_, keys);
    return keys;
}

void PatientBST::collectInorderKeys(BSTNode* node,
                                    std::vector<std::pair<std::string, std::string>>& out) const {
    if (node == nullptr) {
        return;
    }
    collectInorderKeys(node->left, out);
    out.emplace_back(node->hospitalId, node->patientId);
    collectInorderKeys(node->right, out);
}

size_t PatientBST::size() const {
    return size_;
}

bool PatientBST::isEmpty() const {
    return size_ == 0;
}

void PatientBST::clear() {
    destroyTree(root_);
    root_ = nullptr;
    size_ = 0;
}

void PatientBST::destroyTree(BSTNode* node) {
    if (node == nullptr) {
        return;
    }
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}

BSTNode* PatientBST::cloneTree(BSTNode* node) {
    if (node == nullptr) {
        return nullptr;
    }
    BSTNode* newNode = new BSTNode(node->hospitalId, node->patientId);
    newNode->left = cloneTree(node->left);
    newNode->right = cloneTree(node->right);
    return newNode;
}

} // namespace smarter

