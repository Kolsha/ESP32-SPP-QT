#ifndef AVERAGE_BUFFER_H
#define AVERAGE_BUFFER_H

#include <mutex>

template <class T>
class AverageBuffer {
public:
    explicit AverageBuffer(const size_t size = 30, const T & initial_val = 100) :
        m_buf(std::unique_ptr<T[]>(new T[size])),
        m_size(size)
    {
        m_buf[m_tail] = initial_val;
    }

    void put(const T & item)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_buf[m_head] = item;
        m_head = (m_head + 1) % m_size;

        if(m_head == m_tail)
        {
            m_tail = (m_tail + 1) % m_size;
        }
    }

    T getAverage(void)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(empty()){
            return m_buf[m_tail];
        }

        T res = 0;
        size_t real_tail = m_tail;
        size_t count = 0;
        while(!empty())
        {
            auto val = m_buf[m_tail];
            m_tail = (m_tail + 1) % m_size;
            res += val;
            count++;
        }

        res /= count;

        m_tail = real_tail;



        return res;
    }

    void reset(void)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_head = m_tail;
    }

    bool empty(void) const
    {

        return m_head == m_tail;
    }

    bool full(void) const
    {
        //If tail is ahead the head by 1, we are full
        return ((m_head + 1) % m_size) == m_tail;
    }

    size_t size(void) const
    {
        return m_size - 1;
    }

    virtual ~AverageBuffer(){}

private:
    std::mutex m_mutex;
    std::unique_ptr<T[]> m_buf;
    size_t m_head = 0;
    size_t m_tail = 0;
    size_t m_size = 0;
};



#endif // AVERAGE_BUFFER_H
