#include <iostream>
#include <memory_resource>
#include <list>
#include <vector>

#include <cstddef>
#include <cstring>
#include <cassert>
#include <iterator>

class FixedBufferResource : public std::pmr::memory_resource {
private:
    char* buffer;
    std::size_t buffer_size;
    std::size_t used = 0;
    using FreeBlock = std::pair<void*, std::size_t>;
    std::list<FreeBlock> free_list;


    void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override {
        if (!p) return;
        free_list.emplace_back(p, bytes);
    }


    void* do_allocate(std::size_t bytes, std::size_t alignment) override {
        for (auto it = free_list.begin(); it != free_list.end(); ++it) {
            void* ptr = it->first;
            std::size_t block_size = it->second;
            std::size_t space = block_size;


            void* aligned_ptr = std::align(alignment, bytes, ptr, space);


            if (aligned_ptr) {
                std::size_t wasted = static_cast<char*>(aligned_ptr) - static_cast<char*>(ptr);
                std::size_t remaining = block_size - wasted - bytes;
                free_list.erase(it);



                if (remaining > 0) {
                    char* remaining_ptr = static_cast<char*>(aligned_ptr) + bytes;
                    free_list.emplace_back(remaining_ptr, remaining);
                }
                return aligned_ptr;
            }
        }
        std::size_t space = buffer_size - used;
        void* ptr = buffer + used;
        void* aligned_ptr = std::align(alignment, bytes, ptr, space);


        if (!aligned_ptr) {
            throw std::bad_alloc();
        }
        std::size_t wasted = static_cast<char*>(aligned_ptr) - (buffer + used);

        if (used + wasted + bytes > buffer_size) {
            throw std::bad_alloc();
        }

        used += wasted + bytes;
        return aligned_ptr;
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }
public:
    explicit FixedBufferResource(std::size_t size = 1024 * 1024)
        : buffer_size(size), used(0) {
        buffer = static_cast<char*>(::operator new(size));
        if (!buffer) throw std::bad_alloc();
    }

    ~FixedBufferResource() override {
        ::operator delete(buffer);
    }

    FixedBufferResource(const FixedBufferResource&) = delete;
    FixedBufferResource& operator=(const FixedBufferResource&) = delete;
    FixedBufferResource(FixedBufferResource&& other) noexcept
        : buffer(other.buffer), buffer_size(other.buffer_size), used(other.used), free_list(std::move(other.free_list)) {
        other.buffer = nullptr;
        other.buffer_size = 0;
        other.used = 0;
    }

    FixedBufferResource& operator=(FixedBufferResource&& other) noexcept {
        if (this != &other) {
            ::operator delete(buffer);
            buffer = other.buffer;
            buffer_size = other.buffer_size;
            used = other.used;
            free_list = std::move(other.free_list);

            other.buffer = nullptr;
            other.buffer_size = 0;
            other.used = 0;
        }
        return *this;
    }
};

template<typename T>
class Stack {
public:
    using allocator_type = std::pmr::polymorphic_allocator<T>;

private:
    struct Node {
        T value;
        Node* next;

        template<typename U>
        Node(U&& v, Node* n, const allocator_type& alloc)
            : value(std::forward<U>(v)), next(n) {}
    };

    Node* top_node = nullptr;
    allocator_type alloc;

public:
    explicit Stack(const allocator_type& a = allocator_type())
        : alloc(a) {}

    ~Stack() {
        clear();
    }

    void push(const T& value) {
        Node* new_node = alloc.allocate(1);
        try {
            alloc.construct(new_node, value, top_node, alloc);
        } catch (...) {
            alloc.deallocate(new_node, 1);
            throw;
        }
        top_node = new_node;
    }


    void push(T&& value) {
        Node* new_node = alloc.allocate(1);
        try {
            alloc.construct(new_node, std::move(value), top_node, alloc);
        } catch (...) {
            alloc.deallocate(new_node, 1);
            throw;
        }
        top_node = new_node;
    }

    void pop() {
        if (!top_node) return;
        Node* old = top_node;

        top_node = top_node->next;

        alloc.destroy(old);

        alloc.deallocate(old, 1);
    }

    T& top() {
        assert(top_node);
        return top_node->value;
    }

    const T& top() const {
        assert(top_node);
        return top_node->value;
    }

    bool empty() const { return top_node == nullptr; }
    void clear() {
        while (!empty()) pop();
    }

    std::size_t size() const {
        std::size_t cnt = 0;
        for (Node* p = top_node; p; p = p->next) ++cnt;
        return cnt;
    }

    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

    private:
        Node* current;

    public:
        explicit iterator(Node* node = nullptr) : current(node) {}

        reference operator*() const { return current->value; }
        pointer operator->() const { return &(current->value); }

        iterator& operator++() {
            current = current->next;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const { return current == other.current; }
        bool operator!=(const iterator& other) const { return !(*this == other); }
    };

    iterator begin() { return iterator(top_node); }
    iterator end() { return iterator(nullptr); }
};

struct Person {
    std::string name;
    int age;

    Person(std::string n, int a) : name(std::move(n)), age(a) {
        std::cout << "Person(" << name << ") constructed\n";
    }

    ~Person() {
        std::cout << "Person(" << name << ") destroyed\n";
    }
};
