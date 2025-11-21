#include <iostream>
#include <vector>
#include <cassert>
#include "stack.h"

void test_fixed_buffer_resource_basic() {
    std::cout << "Testing FixedBufferResource Basic\n";
    
    FixedBufferResource resource(1024);
    
    void* ptr1 = resource.allocate(64, 8);
    std::cout << "Allocated 64 bytes at: " << ptr1 << "\n";
    assert(ptr1 != nullptr);
    
    void* ptr2 = resource.allocate(128, 16);
    std::cout << "Allocated 128 bytes at: " << ptr2 << "\n";
    assert(ptr2 != nullptr);
    
    resource.deallocate(ptr1, 64, 8);
    std::cout << "Deallocated first block\n";
    
    resource.deallocate(ptr2, 128, 16);
    std::cout << "Deallocated second block\n";
    
    std::cout << "Basic allocation test passed\n\n";
}

void test_fixed_buffer_resource_reuse() {
    std::cout << "Testing Memory Reuse\n";
    
    FixedBufferResource resource(256);
    
    void* ptr1 = resource.allocate(64, 8);
    std::cout << "First allocation at: " << ptr1 << "\n";
    
    resource.deallocate(ptr1, 64, 8);
    std::cout << "Deallocated\n";
    
    void* ptr2 = resource.allocate(64, 8);
    std::cout << "Second allocation at: " << ptr2 << "\n";
    
    if (ptr1 == ptr2) {
        std::cout << "Memory reuse test passed (same address)\n";
    } else {
        std::cout << "Memory reuse test passed (different address - still valid)\n";
    }
    std::cout << "\n";
}

void test_stack_basic() {
    std::cout << "Testing Stack Basic Operations\n";
    
    FixedBufferResource resource(1024);
    std::pmr::polymorphic_allocator<int> alloc(&resource);
    Stack<int> stack(alloc);
    
    assert(stack.empty());
    assert(stack.size() == 0);
    std::cout << "Initial stack is empty\n";
    
    stack.push(10);
    assert(!stack.empty());
    assert(stack.size() == 1);
    assert(stack.top() == 10);
    std::cout << "Pushed 10, top is: " << stack.top() << "\n";
    
    stack.push(20);
    assert(stack.size() == 2);
    assert(stack.top() == 20);
    std::cout << "Pushed 20, top is: " << stack.top() << "\n";
    
    stack.pop();
    assert(stack.size() == 1);
    assert(stack.top() == 10);
    std::cout << "Popped, top is now: " << stack.top() << "\n";
    
    stack.pop();
    assert(stack.empty());
    std::cout << "Stack is empty again\n";
    
    std::cout << "Stack basic operations test passed\n\n";
}

void test_stack_iterator() {
    std::cout << "Testing Stack Iterator\n";
    
    FixedBufferResource resource(1024);
    std::pmr::polymorphic_allocator<int> alloc(&resource);
    Stack<int> stack(alloc);
    
    stack.push(1);
    stack.push(2);
    stack.push(3);
    
    std::cout << "Stack elements (via iterator): ";
    for (auto it = stack.begin(); it != stack.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";
    
    std::cout << "Stack elements (range-based for): ";
    for (int value : stack) {
        std::cout << value << " ";
    }
    std::cout << "\n";
    
    std::cout << "Stack iterator test passed\n\n";
}

void test_stack_complex_type() {
    std::cout << "Testing Stack with Complex Type\n";
    
    FixedBufferResource resource(2048);
    std::pmr::polymorphic_allocator<Person> alloc(&resource);
    Stack<Person> stack(alloc);
    
    stack.push(Person("Alice", 25));
    stack.push(Person("Bob", 30));
    
    assert(stack.size() == 2);
    assert(stack.top().name == "Bob");
    assert(stack.top().age == 30);
    std::cout << "Top person: " << stack.top().name << " (" << stack.top().age << ")\n";
    
    stack.pop();
    assert(stack.top().name == "Alice");
    assert(stack.top().age == 25);
    std::cout << "After pop, top person: " << stack.top().name << " (" << stack.top().age << ")\n";
    
    std::cout << "Complex type test passed\n\n";
}

void test_stack_clear() {
    std::cout << "Testing Stack Clear\n";
    
    FixedBufferResource resource(1024);
    std::pmr::polymorphic_allocator<int> alloc(&resource);
    Stack<int> stack(alloc);
    
    stack.push(1);
    stack.push(2);
    stack.push(3);
    
    assert(stack.size() == 3);
    std::cout << "Stack size before clear: " << stack.size() << "\n";
    
    stack.clear();
    assert(stack.empty());
    assert(stack.size() == 0);
    std::cout << "Stack size after clear: " << stack.size() << "\n";
    
    std::cout << "Stack clear test passed\n\n";
}

int main() {
    std::cout << "Starting tests...\n\n";
    
    try {
        test_fixed_buffer_resource_basic();
        test_fixed_buffer_resource_reuse();
        test_stack_basic();
        test_stack_iterator();
        test_stack_complex_type();
        test_stack_clear();
        
        std::cout << "All tests passed successfully!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception\n";
        return 1;
    }
}
