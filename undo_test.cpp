// undo_test.cpp --- An example of UndoSystem
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>.
#include "UndoSystem.hpp"
#include <iostream>
#include <string>
#include <cassert>

// undo data

struct UndoString;

struct MyUndoData : UndoData
{
    static int s_alive_count;

    MyUndoData()
    {
        ++s_alive_count;
    }

    ~MyUndoData()
    {
        --s_alive_count;
    }

    UndoString *get_base(void *base)
    {
        return static_cast<UndoString *>(base);
    }
};

int MyUndoData::s_alive_count = 0;

struct UndoData_Replace : MyUndoData
{
    std::size_t m_index;
    std::string m_replaced;
    std::string m_with;
    virtual void apply_backward(void *base);
    virtual void apply_forward(void *base);
};

struct UndoData_Erase : MyUndoData
{
    std::size_t m_index;
    std::size_t m_count;
    std::string m_erased;
    virtual void apply_backward(void *base);
    virtual void apply_forward(void *base);
};

struct UndoData_Insert : MyUndoData
{
    std::size_t m_index;
    std::string m_inserted;
    virtual void apply_backward(void *base);
    virtual void apply_forward(void *base);
};

// the main data (base)

struct UndoString : UndoSystem
{
    std::string m_str;

    void print()
    {
        std::cout << "UndoString: '" << m_str << "'" << std::endl;
    }

    // actions without undo info
    void replace(std::size_t index, std::size_t count, const std::string& with);
    void erase(std::size_t index, std::size_t count);
    void insert(std::size_t index, const std::string& inserted);

    // do actions with undo info
    void do_replace(std::size_t index, std::size_t count, const std::string& with);
    void do_erase(std::size_t index, std::size_t count);
    void do_insert(std::size_t index, const std::string& inserted);

    void undo()
    {
        std::cout << "undo" << std::endl;
        UndoSystem::undo(this);
    }
    void redo()
    {
        std::cout << "redo" << std::endl;
        UndoSystem::redo(this);
    }
};

// apply_backward

void UndoData_Replace::apply_backward(void *base)
{
    get_base(base)->replace(m_index, m_with.size(), m_replaced);
}

void UndoData_Erase::apply_backward(void *base)
{
    get_base(base)->insert(m_index, m_erased);
}

void UndoData_Insert::apply_backward(void *base)
{
    get_base(base)->erase(m_index, m_inserted.size());
}

// apply_forward

void UndoData_Replace::apply_forward(void *base)
{
    get_base(base)->replace(m_index, m_replaced.size(), m_with);
}

void UndoData_Erase::apply_forward(void *base)
{
    get_base(base)->erase(m_index, m_count);
}

void UndoData_Insert::apply_forward(void *base)
{
    get_base(base)->insert(m_index, m_inserted);
}

// actions without undo info

void UndoString::replace(std::size_t index, std::size_t count, const std::string& with)
{
    std::cout << "replace(" << index << ", " << count << ", '" << with << "')" << std::endl;
    m_str.replace(index, count, with);
}

void UndoString::erase(std::size_t index, std::size_t count)
{
    std::cout << "erase(" << index << ", " << count << ")" << std::endl;
    m_str.erase(index, count);
}

void UndoString::insert(std::size_t index, const std::string& inserted)
{
    std::cout << "insert(" << index << ", '" << inserted << "')" << std::endl;
    m_str.insert(index, inserted);
}

// do actions with undo info

void UndoString::do_replace(std::size_t index, std::size_t count, const std::string& with)
{
    std::string replaced = m_str.substr(index, count);

    auto undo = new UndoData_Replace();
    undo->m_index = index;
    undo->m_replaced = replaced;
    undo->m_with = with;
    add_undo(undo);

    replace(index, count, with);
}

void UndoString::do_erase(std::size_t index, std::size_t count)
{
    std::string erased = m_str.substr(index, count);

    auto undo = new UndoData_Erase();
    undo->m_index = index;
    undo->m_count = count;
    undo->m_erased = erased;
    add_undo(undo);

    erase(index, count);
}

void UndoString::do_insert(std::size_t index, const std::string& inserted)
{
    auto undo = new UndoData_Insert();
    undo->m_index = index;
    undo->m_inserted = inserted;
    add_undo(undo);

    insert(index, inserted);
}

int main(void)
{
    UndoString str;

    assert(MyUndoData::s_alive_count == 0);

    str.print();
    str.do_insert(0, "ABCDEF");
    str.print();
    str.do_replace(1, 2, "<>");
    str.print();
    str.do_insert(3, "123");
    str.print();
    str.do_erase(0, 3);
    str.print();

    assert(MyUndoData::s_alive_count > 0);

    str.undo();
    str.print();
    str.undo();
    str.print();
    str.redo();
    str.print();
    str.undo();
    str.print();
    str.undo();
    str.print();
    str.undo();
    str.print();

    str.clear_undo_buffer();

    assert(MyUndoData::s_alive_count == 0);

    return 0;
}
