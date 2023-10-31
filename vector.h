#pragma once
#include <cassert>
#include <cstdlib>
#include <new>
#include <utility>
#include <memory>

template <typename T>
class RawMemory {
public:
    RawMemory() = default;

    explicit RawMemory(size_t capacity);        
    RawMemory(const RawMemory&) = delete;
    RawMemory& operator=(const RawMemory& rhs) = delete;
    RawMemory(RawMemory&& other) noexcept;
    RawMemory& operator=(RawMemory&& rhs) noexcept;
    ~RawMemory();

    T* operator+(size_t offset) noexcept;
    const T* operator+(size_t offset) const noexcept;
    const T& operator[](size_t index) const noexcept;
    T& operator[](size_t index) noexcept;
    void Swap(RawMemory& other) noexcept;
    const T* GetAddress() const noexcept;
    T* GetAddress() noexcept;
    size_t Capacity() const;

private:
    // Выделяет сырую память под n элементов и возвращает указатель на неё
    static T* Allocate(size_t n);
    // Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
    static void Deallocate(T* buf) noexcept;

    T* buffer_ = nullptr;
    size_t capacity_ = 0;
};


template <typename T>
class Vector {
public:
    using iterator = T*;
    using const_iterator = const T*;

    iterator begin() noexcept;
    iterator end() noexcept;
    const_iterator begin() const noexcept;
    const_iterator end()const  noexcept;
    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;

    Vector() = default;
    explicit Vector(size_t size);
    explicit Vector(const Vector& other);
    Vector(Vector&& other) noexcept;

    Vector& operator=(const Vector& rhs);
    Vector& operator=(Vector&& rhs) noexcept;

    ~Vector();

    void Resize(size_t new_size);
    void PushBack(const T& value);
    void PushBack(T&& value);

    template <typename... Args>
    T& EmplaceBack(Args&&... args);
   
    template <typename... Args>
    iterator Emplace(const_iterator pos, Args&&... args);
   
    iterator Erase(const_iterator pos) noexcept(std::is_nothrow_move_assignable_v<T>);    
    iterator Insert(const_iterator pos, const T& value);
    iterator Insert(const_iterator pos, T&& value);
    void PopBack()  noexcept;
    void Reserve(size_t new_capacity);
    size_t Size() const noexcept;
    size_t Capacity() const noexcept;
    const T& operator[](size_t index) const noexcept;
    T& operator[](size_t index) noexcept;
    void Swap(Vector& other) noexcept;
    

private:
    size_t size_ = 0;    
    RawMemory<T> data_; 
   
};




template<typename T>
inline T* Vector<T>::begin() noexcept
{
    return data_.GetAddress();
}

template<typename T>
inline T* Vector<T>::end() noexcept
{
    return data_.GetAddress() + size_;
}

template<typename T>
inline const T* Vector<T>::begin() const noexcept
{
    return const_iterator(data_.GetAddress());
}

template<typename T>
inline const T* Vector<T>::end() const noexcept
{
    return const_iterator(data_.GetAddress() + size_);
}

template<typename T>
inline const T* Vector<T>::cbegin() const noexcept
{
    return begin();
}

template<typename T>
inline const T* Vector<T>::cend() const noexcept
{
    return end();
}

template<typename T>
inline Vector<T>::Vector(size_t size) :
size_(size),
data_(size)
{
    std::uninitialized_value_construct_n(data_.GetAddress(), size);
}

template<typename T>
inline Vector<T>::Vector(const Vector& other) :
    size_(other.size_),
    data_(other.size_)
{
    std::uninitialized_copy_n(other.data_.GetAddress(), size_, data_.GetAddress());
}

template<typename T>
inline Vector<T>::Vector(Vector&& other) noexcept
{
    Swap(other);
}

template<typename T>
inline Vector<T>& Vector<T>::operator=(const Vector& rhs)
{
    if (this != &rhs) {
        if (rhs.size_ > data_.Capacity()) {
            Vector rhs_copy(rhs);
            Swap(rhs_copy);
        }
        else {
            if (size_ > rhs.size_) {
                std::copy(rhs.data_.GetAddress(), rhs.data_.GetAddress() + rhs.size_, data_.GetAddress());
                std::destroy_n(data_.GetAddress() + rhs.size_, size_ - rhs.size_);
            }
            else {
                std::copy(rhs.data_.GetAddress(), rhs.data_.GetAddress() + size_, data_.GetAddress());
                std::uninitialized_copy_n(rhs.data_.GetAddress() + size_, rhs.size_ - size_, data_.GetAddress() + size_);
            }
            size_ = rhs.size_;

        }
    }
    return *this;
}

template<typename T>
inline Vector<T>& Vector<T>::operator=(Vector&& rhs) noexcept
{
    if (this != &rhs) {
        Swap(rhs);
    }
    return *this;
}

template<typename T>
inline Vector<T>::~Vector()
{
    std::destroy_n(data_.GetAddress(), size_);
}

template<typename T>
inline void Vector<T>::Resize(size_t new_size)
{
    if (new_size != size_) {
        if (new_size < size_) {
            std::destroy_n(data_.GetAddress() + new_size, size_ - new_size);
            size_ = new_size;
        }
        else {
            Reserve(new_size);
            std::uninitialized_value_construct_n(data_.GetAddress() + size_, new_size - size_);
            size_ = new_size;

        }
    }
}

template<typename T>
inline void Vector<T>::PushBack(const T& value)
{
    EmplaceBack(std::forward<const T&>(value));
}

template<typename T>
inline void Vector<T>::PushBack(T&& value)
{
    EmplaceBack(std::forward<T&&>(value));
}

template<typename T>
inline T* Vector<T>::Erase(const_iterator pos) noexcept(std::is_nothrow_move_assignable_v<T>)
{
    size_t index = pos - begin();
    std::move(begin() + index + 1, end(), begin() + index);
    PopBack();
    return begin() + index;
}

template<typename T>
inline T* Vector<T>::Insert(const_iterator pos, const T& value)
{
    return Emplace(pos, std::forward<const T&>(value));
}

//?
template<typename T>
inline T* Vector<T>::Insert(const_iterator pos, T&& value)
{
    return Emplace(pos, std::forward<T&&>(value));
}

template<typename T>
inline void Vector<T>::PopBack() noexcept
{
    if (size_ > 0) {
        std::destroy_at(data_.GetAddress() + size_ - 1);
        --size_;
    }
}

template<typename T>
inline void Vector<T>::Reserve(size_t new_capacity)
{
    if (new_capacity <= data_.Capacity()) {
        return;
    }
    RawMemory<T> new_data(new_capacity);
    if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
        std::uninitialized_move_n(data_.GetAddress(), size_, new_data.GetAddress());
    }
    else {
        std::uninitialized_copy_n(data_.GetAddress(), size_, new_data.GetAddress());
    }
    std::destroy_n(data_.GetAddress(), size_);
    data_.Swap(new_data);
}

template<typename T>
inline size_t Vector<T>::Size() const noexcept
{
    return size_;
}

template<typename T>
inline size_t Vector<T>::Capacity() const noexcept
{
    return data_.Capacity();
}

template<typename T>
inline const T& Vector<T>::operator[](size_t index) const noexcept
{
    return const_cast<Vector&>(*this)[index];
}

template<typename T>
inline T& Vector<T>::operator[](size_t index) noexcept
{
    assert(index < size_);
    return data_[index];
}

template<typename T>
inline void Vector<T>::Swap(Vector& other) noexcept
{
    data_.Swap(other.data_);
    std::swap(size_, other.size_);
}

template<typename T>
template<typename ...Args>
inline T& Vector<T>::EmplaceBack(Args && ...args)
{
    if (size_ < Capacity()) {
        new (data_ + size_) T(std::forward<Args>(args)...);
    }
    else {
        RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
        new (new_data + size_) T(std::forward<Args>(args)...);
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
            std::uninitialized_move_n(data_.GetAddress(), size_, new_data.GetAddress());
        }
        else {
            std::uninitialized_copy_n(data_.GetAddress(), size_, new_data.GetAddress());
        }
        std::destroy_n(data_.GetAddress(), size_);
        data_.Swap(new_data);
    }
    ++size_;
    return data_[size_ - 1];
}

template<typename T>
template<typename ...Args>
inline T* Vector<T>::Emplace(const_iterator pos, Args && ...args)
{
    size_t index = pos - begin();
    iterator result = nullptr;
    if (size_ != Capacity()) {
        //если есть место            
        if (size_ != 0) {
            new (data_ + size_) T(std::move(*(data_.GetAddress() + size_ - 1)));
            try {
                std::move_backward(begin() + index, data_.GetAddress() + size_, data_.GetAddress() + size_ + 1);
            }
            catch (...) {
                std::destroy_n(data_.GetAddress() + size_, 1);
                throw;
            }
            std::destroy_at(begin() + index);
        }
        result = new (data_ + index) T(std::forward < Args>(args)...);

    }
    else {
        // если нет места
        RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
        result = new (new_data + index) T(std::forward<Args>(args)...);
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
            std::uninitialized_move_n(data_.GetAddress(), index, new_data.GetAddress());
            std::uninitialized_move_n(data_.GetAddress() + index, size_ - index, new_data.GetAddress() + index + 1);
        }
        else {
            try {
                std::uninitialized_copy_n(data_.GetAddress(), index, new_data.GetAddress());
                std::uninitialized_copy_n(data_.GetAddress() + index, size_ - index, new_data.GetAddress() + index + 1);
            }
            catch (...) {
                std::destroy_n(new_data.GetAddress() + index, 1);
                throw;
            }
        }
        std::destroy_n(begin(), size_);
        data_.Swap(new_data);
    }
    ++size_;
    return result;
}

template<typename T>
inline RawMemory<T>::RawMemory(size_t capacity)
    : buffer_(Allocate(capacity))
    , capacity_(capacity)
{}

template<typename T>
inline RawMemory<T>::RawMemory(RawMemory && other) noexcept
    :
    buffer_(std::move(other.buffer_)),
    capacity_(std::move(other.capacity_))
{}

template<typename T>
inline RawMemory<T>& RawMemory<T>::operator=(RawMemory && rhs) noexcept
{
    buffer_ = std::move(rhs.buffer_);
    capacity_ = std::move(rhs.capacity_);
    return *this;
}

template<typename T>
inline RawMemory<T>::~RawMemory()
{
    Deallocate(buffer_);
}

template<typename T>
inline T* RawMemory<T>::operator+(size_t offset) noexcept
{
    // Разрешается получать адрес ячейки памяти, следующей за последним элементом массива
    assert(offset <= capacity_);
    return buffer_ + offset;
}

template<typename T>
inline const T* RawMemory<T>::operator+(size_t offset) const noexcept
{
    return const_cast<RawMemory&>(*this) + offset;
}

template<typename T>
inline const T& RawMemory<T>::operator[](size_t index) const noexcept
{
    return const_cast<RawMemory&>(*this)[index];
}

template<typename T>
inline T& RawMemory<T>::operator[](size_t index) noexcept
{
    assert(index < capacity_);
    return buffer_[index];
}

template<typename T>
inline void RawMemory<T>::Swap(RawMemory& other) noexcept
{
    std::swap(buffer_, other.buffer_);
    std::swap(capacity_, other.capacity_);
}

template<typename T>
inline const T* RawMemory<T>::GetAddress() const noexcept
{
    return buffer_;
}

template<typename T>
inline T* RawMemory<T>::GetAddress() noexcept
{
    return buffer_;
}

template<typename T>
inline size_t RawMemory<T>::Capacity() const
{
    return capacity_;
}

template<typename T>
inline T* RawMemory<T>::Allocate(size_t n)
{
    return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
}

template<typename T>
inline void RawMemory<T>::Deallocate(T* buf) noexcept
{
    operator delete(buf);
}