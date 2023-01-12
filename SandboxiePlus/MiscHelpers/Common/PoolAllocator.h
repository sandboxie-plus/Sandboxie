# pragma once

template<size_t ChunkSize>
class PoolAllocator
{
    struct Page
    {
        char memory[4096];
    };

    union Node
    {
         Node *previous;
         char chunk[ChunkSize];
    };
public:
    PoolAllocator()
    {
    }
    ~PoolAllocator()
    {
        dispose();
    }
    void *allocate(size_t size)
    {
        Q_ASSERT(size <= ChunkSize);
        if (!stack && !grow())
            return nullptr;
        Node *top = stack;
        stack = stack->previous;
        return top->chunk;
    }
    void free(void *ptr)
    {
        Node *top = static_cast<Node*>(ptr);
        top->previous = stack;
        stack = top;
    }
    void dispose() {
        for (auto page : pages)
            ::free(page);
        pages.clear();
        stack = nullptr;
    }

private:
    bool grow()
    {
        Page *newPage = static_cast<Page*>(::malloc(sizeof(Page)));
        if (!newPage)
            return false;
        initPage(newPage);
        pages.append(newPage);
        return true;
    }
    void initPage(Page *page)
    {
        Node *newStack = reinterpret_cast<Node*>(page->memory);
        newStack->previous = stack;

        const size_t numChunks = sizeof(Page) / sizeof(Node);
        for (size_t i = 0; i < numChunks - 1; ++i, ++newStack)
            newStack[1].previous = newStack;
        stack = newStack;
    }

    Node *stack = nullptr;
    QVector<Page*> pages;
};