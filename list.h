#pragma once

template <typename T, typename Allocator = std::allocator<T>>
class List {
  private:
    struct BaseNode {
        BaseNode* prev = nullptr;
        BaseNode* next = nullptr;

        BaseNode() = default;
        BaseNode(BaseNode* pr, BaseNode* ne)
            : prev(pr), next(ne) {}
    };

    struct Node : BaseNode {
        T value;

        Node() = default;
        Node(const T& val)
            : value(val) {}
        Node(BaseNode* pr, BaseNode* ne)
            : BaseNode(pr, ne) {}
        Node(BaseNode* pr, BaseNode* ne, const T& val)
            : BaseNode(pr, ne), value(val) {}
    };

    BaseNode fakeNode;
    size_t sz;

    using AllocatorTraits = typename std::allocator_traits<Allocator>;
    using NodeAllocator = typename AllocatorTraits::template rebind_alloc<Node>;
    using NodeAllocatorTraits = typename std::allocator_traits<NodeAllocator>;

    Allocator allocator;
    NodeAllocator nodeAllocator;

    Node* allocateNode() {
        Node* nw = NodeAllocatorTraits::allocate(nodeAllocator, 1);
        return nw;
    }

    void constructNode(Node* ptr, BaseNode* pr, BaseNode* ne) {
        NodeAllocatorTraits::construct(nodeAllocator, ptr, pr, ne);
    }

    void constructNode(Node* ptr, BaseNode* pr, BaseNode* ne, const T& val) {
        NodeAllocatorTraits::construct(nodeAllocator, ptr, pr, ne, val);
    }

    void deleteNode(Node* ptr) {
        NodeAllocatorTraits::destroy(nodeAllocator, ptr);
        NodeAllocatorTraits::deallocate(nodeAllocator, ptr, 1);
    }

    void makeFakeNode() {
        // fakeNode = static_cast<BaseNode*>(NodeAllocatorTraits::allocate(nodeAllocator, 1));
        // fakeNode->prev = fakeNode;
        // fakeNode->next = fakeNode;
        fakeNode.prev = &fakeNode;
        fakeNode.next = &fakeNode;
    }

    void clear(size_t i) {
        BaseNode* st = fakeNode.next;
        for (size_t j = 0; j < i; ++j) {
            BaseNode* nxt = st->next;
            deleteNode(static_cast<Node*>(st));
            st = nxt;
        }
        sz = 0;
    }

  public:
    // List(): sz(0) { makeFakeNode(); }

    List(size_t n, const T& val, Allocator alloc = Allocator())
        : sz(n), allocator(alloc), nodeAllocator(alloc) {
        makeFakeNode();
        BaseNode* last = &fakeNode;
        for (size_t i = 0; i < n; ++i) {
            Node* to_add = allocateNode();
            try {
                constructNode(to_add, last, &fakeNode, val);
            } catch (...) {
                NodeAllocatorTraits::deallocate(nodeAllocator, to_add, 1);
                clear(i);
                throw;
            }
            last->next = to_add;
            fakeNode.prev = to_add;
            last = to_add;
        }
        fakeNode.prev = last;
    }

    List(const Allocator& alloc = Allocator())
        : sz(0), allocator(alloc), nodeAllocator(alloc) {
        makeFakeNode();
    }

    List(size_t n, Allocator alloc = Allocator())
        : sz(n), allocator(alloc), nodeAllocator(alloc) {
        makeFakeNode();
        BaseNode* last = &fakeNode;
        for (size_t i = 0; i < n; ++i) {
            Node* to_add = allocateNode();
            try {
                constructNode(to_add, last, &fakeNode);
            } catch (...) {
                NodeAllocatorTraits::deallocate(nodeAllocator, to_add, 1);
                clear(i);
                throw;
            }
            last->next = to_add;
            fakeNode.prev = to_add;
            last = to_add;
        }
        fakeNode.prev = last;
    }

    List(const List& other)
        : sz(other.size()) {
        allocator = AllocatorTraits::select_on_container_copy_construction(other.get_allocator());
        nodeAllocator = allocator;
        makeFakeNode();
        BaseNode* last = &fakeNode;
        const_iterator cur = other.cbegin();
        for (size_t i = 0; i < sz; ++i) {
            Node* to_add = allocateNode();
            try {
                constructNode(to_add, last, &fakeNode, *cur);
            } catch (...) {
                NodeAllocatorTraits::deallocate(nodeAllocator, to_add, 1);
                clear(i);
                throw;
            }
            ++cur;
            last->next = to_add;
            fakeNode.prev = to_add;
            last = to_add;
        }
        fakeNode.prev = last;
    }

    List& operator=(const List& other) {
        if (AllocatorTraits::propagate_on_container_copy_assignment::value) {
            allocator = other.get_allocator();
            nodeAllocator = allocator;
        }
        List copy(allocator);
        const_iterator cur = other.cbegin();
        for (size_t i = 0; i < other.sz; ++i) {
            copy.push_back(*cur);
            ++cur;
        }
        std::swap(fakeNode.next, copy.fakeNode.next);
        std::swap(fakeNode.prev, copy.fakeNode.prev);
        std::swap(sz, copy.sz);
        return *this;
    }

    ~List() {
        clear(sz);
    }

    Allocator get_allocator() const {
        return allocator;
    }

    size_t size() const {
        return sz;
    }

    template <bool is_const>
    class common_iterator {
      private:
      public:
        using value_type = std::conditional_t<is_const, const T, T>;
        using reference = std::conditional_t<is_const, const T&, T&>;
        using pointer = std::conditional_t<is_const, const T*, T*>;
        using base_node_pointer = std::conditional_t<is_const, const BaseNode*, BaseNode*>;
        using node_pointer = std::conditional_t<is_const, const Node*, Node*>;

        using difference_type = int;
        using iterator_category = std::bidirectional_iterator_tag;

        base_node_pointer ptr;

        common_iterator()
            : ptr(nullptr) {}
        common_iterator(base_node_pointer ptr_)
            : ptr(ptr_) {}
        common_iterator(const common_iterator& other)
            : ptr(other.ptr) {}

        common_iterator& operator=(const common_iterator& other) {
            ptr = other.ptr;
            return *this;
        }

        common_iterator& operator++() {
            ptr = ptr->next;
            return *this;
        }

        common_iterator& operator--() {
            ptr = ptr->prev;
            return *this;
        }

        common_iterator operator++(int) {
            common_iterator cpy = *this;
            ptr = ptr->next;
            return cpy;
        }

        common_iterator operator--(int) {
            common_iterator cpy = *this;
            ptr = ptr->prev;
            return cpy;
        }

        reference operator*() const {
            return static_cast<node_pointer>(ptr)->value;
        }

        bool operator==(const common_iterator& other) const {
            return (ptr == other.ptr);
        }

        bool operator!=(const common_iterator& other) const {
            return !(ptr == other.ptr);
        }

        operator common_iterator<true>() const {
            return common_iterator<true>(ptr);
        }
    };

    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() {
        return iterator(fakeNode.next);
    }
    const_iterator begin() const {
        return cbegin();
    }
    const_iterator cbegin() const {
        return const_iterator(fakeNode.next);
    }

    iterator end() {
        return iterator(&fakeNode);
    }
    const_iterator end() const {
        return cend();
    }
    const_iterator cend() const {
        return const_iterator(&fakeNode);
    }

    reverse_iterator rbegin() {
        return std::reverse_iterator(&fakeNode);
    }
    const_reverse_iterator rbegin() const {
        return crbegin();
    }
    reverse_iterator rend() {
        return std::reverse_iterator(fakeNode.next);
    }
    const_reverse_iterator rend() const {
        return crend();
    }

    const_reverse_iterator crbegin() const {
        return std::make_reverse_iterator(cend());
    }
    const_reverse_iterator crend() const {
        return std::make_reverse_iterator(cbegin());
    }

    void insert(const_iterator it, const T& val) {
        Node* to_add = allocateNode();

        BaseNode* pre = const_cast<BaseNode*>(it.ptr->prev);
        BaseNode* nxt = const_cast<BaseNode*>(it.ptr);

        try {
            constructNode(to_add, pre, nxt, val);
        } catch (...) {
            NodeAllocatorTraits::deallocate(nodeAllocator, to_add, 1);
            throw;
        }

        pre->next = to_add;
        nxt->prev = to_add;
        ++sz;
    }

    void erase(const_iterator it) {
        BaseNode* pre = const_cast<BaseNode*>(it.ptr->prev);
        BaseNode* nxt = const_cast<BaseNode*>(it.ptr->next);

        pre->next = nxt;
        nxt->prev = pre;
        deleteNode(static_cast<Node*>(const_cast<BaseNode*>(it.ptr)));
        --sz;
    }

    void push_back(const T& val) {
        insert(end(), val);
    }
    void push_front(const T& val) {
        insert(begin(), val);
    }
    void pop_back() {
        erase(--end());
    }
    void pop_front() {
        erase(begin());
    }
};
