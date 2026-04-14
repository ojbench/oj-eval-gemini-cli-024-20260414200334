#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"
#include <cstddef>
#include <iterator>

namespace sjtu {

template<class T>
class deque {
private:
    static const size_t BLOCK_SIZE = 512;
    size_t map_capacity;
    T** map;
    size_t head_block;
    size_t head_pos;
    size_t count;

    size_t get_used_blocks() const {
        return count == 0 ? 0 : (head_pos + count - 1) / BLOCK_SIZE + 1;
    }

    void double_map_capacity() {
        size_t new_capacity = map_capacity == 0 ? 8 : map_capacity * 2;
        T** new_map = new T*[new_capacity];
        for (size_t i = 0; i < new_capacity; ++i) new_map[i] = nullptr;
        size_t used_blocks = get_used_blocks();
        for (size_t i = 0; i < used_blocks; ++i) {
            new_map[i] = map[(head_block + i) % map_capacity];
        }
        for (size_t i = used_blocks; i < map_capacity; ++i) {
            if (map[(head_block + i) % map_capacity] != nullptr) {
                delete[] reinterpret_cast<char*>(map[(head_block + i) % map_capacity]);
            }
        }
        delete[] map;
        map = new_map;
        head_block = 0;
        map_capacity = new_capacity;
    }

public:
    class const_iterator;
    class iterator {
        friend class deque;
    private:
        deque* q;
        size_t index;
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::random_access_iterator_tag;

        iterator(deque* q = nullptr, size_t index = 0) : q(q), index(index) {}

        iterator operator+(const int &n) const {
            return iterator(q, index + n);
        }
        iterator operator-(const int &n) const {
            return iterator(q, index - n);
        }
        int operator-(const iterator &rhs) const {
            if (q != rhs.q) throw invalid_iterator();
            return index - rhs.index;
        }
        iterator operator+=(const int &n) {
            index += n;
            return *this;
        }
        iterator operator-=(const int &n) {
            index -= n;
            return *this;
        }
        iterator operator++(int) {
            iterator tmp = *this;
            index++;
            return tmp;
        }
        iterator& operator++() {
            index++;
            return *this;
        }
        iterator operator--(int) {
            iterator tmp = *this;
            index--;
            return tmp;
        }
        iterator& operator--() {
            index--;
            return *this;
        }
        T& operator*() const {
            if (index >= q->count) throw invalid_iterator();
            return (*q)[index];
        }
        T* operator->() const noexcept {
            return &((*q)[index]);
        }
        bool operator==(const iterator &rhs) const {
            return q == rhs.q && index == rhs.index;
        }
        bool operator==(const const_iterator &rhs) const {
            return q == rhs.q && index == rhs.index;
        }
        bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }
        bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }
    };

    class const_iterator {
        friend class deque;
    private:
        const deque* q;
        size_t index;
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = const T;
        using pointer = const T*;
        using reference = const T&;
        using iterator_category = std::random_access_iterator_tag;

        const_iterator(const deque* q = nullptr, size_t index = 0) : q(q), index(index) {}
        const_iterator(const iterator &other) : q(other.q), index(other.index) {}

        const_iterator operator+(const int &n) const {
            return const_iterator(q, index + n);
        }
        const_iterator operator-(const int &n) const {
            return const_iterator(q, index - n);
        }
        int operator-(const const_iterator &rhs) const {
            if (q != rhs.q) throw invalid_iterator();
            return index - rhs.index;
        }
        const_iterator operator+=(const int &n) {
            index += n;
            return *this;
        }
        const_iterator operator-=(const int &n) {
            index -= n;
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator tmp = *this;
            index++;
            return tmp;
        }
        const_iterator& operator++() {
            index++;
            return *this;
        }
        const_iterator operator--(int) {
            const_iterator tmp = *this;
            index--;
            return tmp;
        }
        const_iterator& operator--() {
            index--;
            return *this;
        }
        const T& operator*() const {
            if (index >= q->count) throw invalid_iterator();
            return (*q)[index];
        }
        const T* operator->() const noexcept {
            return &((*q)[index]);
        }
        bool operator==(const iterator &rhs) const {
            return q == rhs.q && index == rhs.index;
        }
        bool operator==(const const_iterator &rhs) const {
            return q == rhs.q && index == rhs.index;
        }
        bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }
        bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }
    };

    deque() : map_capacity(0), map(nullptr), head_block(0), head_pos(0), count(0) {}

    deque(const deque &other) : map_capacity(0), map(nullptr), head_block(0), head_pos(0), count(0) {
        for (size_t i = 0; i < other.count; ++i) {
            push_back(other[i]);
        }
    }

    ~deque() {
        clear();
        for (size_t i = 0; i < map_capacity; ++i) {
            if (map[i] != nullptr) {
                delete[] reinterpret_cast<char*>(map[i]);
            }
        }
        delete[] map;
    }

    deque &operator=(const deque &other) {
        if (this == &other) return *this;
        clear();
        for (size_t i = 0; i < other.count; ++i) {
            push_back(other[i]);
        }
        return *this;
    }

    T & at(const size_t &pos) {
        if (pos >= count) throw index_out_of_bound();
        size_t offset = head_pos + pos;
        size_t block_idx = (head_block + offset / BLOCK_SIZE) % map_capacity;
        size_t pos_in_block = offset % BLOCK_SIZE;
        return map[block_idx][pos_in_block];
    }

    const T & at(const size_t &pos) const {
        if (pos >= count) throw index_out_of_bound();
        size_t offset = head_pos + pos;
        size_t block_idx = (head_block + offset / BLOCK_SIZE) % map_capacity;
        size_t pos_in_block = offset % BLOCK_SIZE;
        return map[block_idx][pos_in_block];
    }

    T & operator[](const size_t &pos) {
        if (pos >= count) throw index_out_of_bound();
        size_t offset = head_pos + pos;
        size_t block_idx = (head_block + offset / BLOCK_SIZE) % map_capacity;
        size_t pos_in_block = offset % BLOCK_SIZE;
        return map[block_idx][pos_in_block];
    }

    const T & operator[](const size_t &pos) const {
        if (pos >= count) throw index_out_of_bound();
        size_t offset = head_pos + pos;
        size_t block_idx = (head_block + offset / BLOCK_SIZE) % map_capacity;
        size_t pos_in_block = offset % BLOCK_SIZE;
        return map[block_idx][pos_in_block];
    }

    const T & front() const {
        if (count == 0) throw container_is_empty();
        return (*this)[0];
    }

    const T & back() const {
        if (count == 0) throw container_is_empty();
        return (*this)[count - 1];
    }

    iterator begin() { return iterator(this, 0); }
    const_iterator cbegin() const { return const_iterator(this, 0); }
    iterator end() { return iterator(this, count); }
    const_iterator cend() const { return const_iterator(this, count); }

    bool empty() const { return count == 0; }
    size_t size() const { return count; }

    void clear() {
        while (count > 0) pop_back();
    }

    iterator insert(iterator pos, const T &value) {
        size_t index = pos.index;
        if (pos.q != this || index > count) throw invalid_iterator();
        if (index == count) {
            push_back(value);
            return iterator(this, count - 1);
        }
        T copy_val = value;
        push_back((*this)[count - 1]);
        for (size_t i = count - 2; i > index; --i) {
            (*this)[i] = std::move((*this)[i - 1]);
        }
        (*this)[index] = std::move(copy_val);
        return iterator(this, index);
    }

    iterator erase(iterator pos) {
        size_t index = pos.index;
        if (pos.q != this || index >= count) throw invalid_iterator();
        if (index == count - 1) {
            pop_back();
            return iterator(this, count);
        }
        for (size_t i = index; i < count - 1; ++i) {
            (*this)[i] = std::move((*this)[i + 1]);
        }
        pop_back();
        return iterator(this, index);
    }

    void push_back(const T &value) {
        if (map_capacity == 0 || (head_pos + count) / BLOCK_SIZE >= map_capacity) {
            double_map_capacity();
        }
        size_t offset = head_pos + count;
        size_t block_idx = (head_block + offset / BLOCK_SIZE) % map_capacity;
        size_t pos_in_block = offset % BLOCK_SIZE;
        if (map[block_idx] == nullptr) {
            map[block_idx] = reinterpret_cast<T*>(new char[BLOCK_SIZE * sizeof(T)]);
        }
        new (&map[block_idx][pos_in_block]) T(value);
        count++;
    }

    void pop_back() {
        if (count == 0) throw container_is_empty();
        size_t offset = head_pos + count - 1;
        size_t block_idx = (head_block + offset / BLOCK_SIZE) % map_capacity;
        size_t pos_in_block = offset % BLOCK_SIZE;
        map[block_idx][pos_in_block].~T();
        if (pos_in_block == 0) {
            delete[] reinterpret_cast<char*>(map[block_idx]);
            map[block_idx] = nullptr;
        }
        count--;
    }

    void push_front(const T &value) {
        if (map_capacity == 0 || (head_pos == 0 && get_used_blocks() >= map_capacity)) {
            double_map_capacity();
        }
        if (head_pos == 0) {
            head_block = (head_block - 1 + map_capacity) % map_capacity;
            head_pos = BLOCK_SIZE - 1;
        } else {
            head_pos--;
        }
        if (map[head_block] == nullptr) {
            map[head_block] = reinterpret_cast<T*>(new char[BLOCK_SIZE * sizeof(T)]);
        }
        new (&map[head_block][head_pos]) T(value);
        count++;
    }

    void pop_front() {
        if (count == 0) throw container_is_empty();
        map[head_block][head_pos].~T();
        head_pos++;
        if (head_pos == BLOCK_SIZE) {
            delete[] reinterpret_cast<char*>(map[head_block]);
            map[head_block] = nullptr;
            head_block = (head_block + 1) % map_capacity;
            head_pos = 0;
        }
        count--;
    }
};

}

#endif