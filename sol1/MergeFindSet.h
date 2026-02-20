#pragma once
#include <stdexcept>

#define MAX 100000

class MergeFindSet
{
private:
    unsigned max_element;
    unsigned *tree;
    unsigned *rank;

    void unite(unsigned root1, unsigned root2)
    {
        if (rank[root1] > rank[root2])
            tree[root2] = root1;
        else
            tree[root1] = root2;
        if (rank[root1] == rank[root2])
            ++rank[root2];
    }

public:
    MergeFindSet(unsigned _max_element) : max_element{ _max_element }
    {
        if (max_element == 0 || max_element > MAX)
            throw std::runtime_error("Number outside accepted range!");
        tree = new unsigned[max_element + 1];
        rank = new unsigned[max_element + 1];
        if (tree == nullptr || rank == nullptr)
            throw std::runtime_error("Can't allocate the required memory!");
        for (unsigned index = 1; index <= max_element; ++index)
        {
            tree[index] = index;
            rank[index] = 1;
        }
    }

    ~MergeFindSet()
    {
        delete[] tree;
        delete[] rank;
    }

    unsigned find(unsigned element)
    {
        unsigned root;
        for (root = element; tree[root] != root; root = tree[root]);
        unsigned aux;
        while (tree[element] != element)
        {
            aux = tree[element];
            tree[element] = root;
            element = aux;
        }
        return root;
    }

    bool are_same_set(unsigned element1, unsigned element2)
    {
        if (element1 == 0 || element1 > MAX)
            throw std::runtime_error("element1 outside accepted range!");
        if (element2 == 0 || element2 > MAX)
            throw std::runtime_error("element2 outside accepted range!");
        if (element1 == element2)
            return true;
        return find(element1) == find(element2);
    }

    void merge_sets(unsigned element1, unsigned element2)
    {
        if (element1 == 0 || element1 > MAX)
            throw std::runtime_error("element1 outside accepted range!");
        if (element2 == 0 || element2 > MAX)
            throw std::runtime_error("element2 outside accepted range!");
        unsigned root1 = find(element1);
        unsigned root2 = find(element2);
        if (root1 == root2)
            throw std::runtime_error("Elements belong to the same set already!");
        unite(root1, root2);
    }
};