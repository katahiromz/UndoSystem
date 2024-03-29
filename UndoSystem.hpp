// UndoSystem.hpp --- Undo/Redo framework for C++11
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>.
#ifndef UNDO_SYSTEM_HPP_
#define UNDO_SYSTEM_HPP_    6   // Version 6

#include <deque>    // for std::deque
#include <memory>   // for std::shared_ptr

struct UndoData
{
    virtual void apply_backward(void *base) = 0;    // for undo
    virtual void apply_forward(void *base) = 0;     // for redo

    virtual ~UndoData() { }
};

struct UndoSystem
{
    typedef std::deque<std::shared_ptr<UndoData>> undo_buffer_t;
    std::size_t m_undo_index;
    undo_buffer_t m_undo_buffer;

    UndoSystem() : m_undo_index(0)
    {
    }

    bool can_undo() const
    {
        return m_undo_index > 0;
    }

    bool can_redo() const
    {
        return m_undo_index < m_undo_buffer.size();
    }

    bool undo(void *base)
    {
        if (can_undo())
        {
            --m_undo_index;
            m_undo_buffer[m_undo_index]->apply_backward(base);
            return true;
        }
        return false;
    }

    bool redo(void *base)
    {
        if (can_redo())
        {
            m_undo_buffer[m_undo_index]->apply_forward(base);
            ++m_undo_index;
            return true;
        }
        return false;
    }

    virtual std::size_t get_undo_max() const
    {
        return 256;
    }

    void check_undo_max()
    {
        if (m_undo_index > 0 &&
            m_undo_buffer.size() > get_undo_max())
        {
            m_undo_buffer.pop_front();
            --m_undo_index;
        }
    }

    void add_undo(UndoData *data)
    {
        m_undo_buffer.resize(m_undo_index);
        m_undo_buffer.emplace_back(data);
        ++m_undo_index;
        check_undo_max();
    }

    void add_undo(std::shared_ptr<UndoData> data)
    {
        m_undo_buffer.resize(m_undo_index);
        m_undo_buffer.push_back(data);
        ++m_undo_index;
        check_undo_max();
    }

    void clear_undo_buffer()
    {
        m_undo_index = 0;
        m_undo_buffer.clear();
    }
};

#endif  // ndef UNDO_SYSTEM_HPP_
