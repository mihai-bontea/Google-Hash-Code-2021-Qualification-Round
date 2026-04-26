#include <vector>
#include <cstdint>
#include <stdexcept>

template <typename T, int MAX_P = 10000>
class BucketQueue{
private:
    static constexpr int WORD_SIZE = 64;
    static constexpr int NUM_BUCKETS = MAX_P + 1;
    static constexpr int NUM_WORDS = (NUM_BUCKETS + WORD_SIZE - 1) / WORD_SIZE;

    std::vector<T> buckets[NUM_BUCKETS];
    uint64_t bitset[NUM_WORDS] = {};

    int size = 0;

    static inline int word_index(int p) { return p / WORD_SIZE; }
    static inline int bit_index(int p) { return p % WORD_SIZE; }

    inline void set_bit(int p) noexcept
    {
        bitset[word_index(p)] |= (1ULL << bit_index(p));
    }

    inline void clear_bit(int p) noexcept
    {
        bitset[word_index(p)] &= ~(1ULL << bit_index(p));
    }

    inline bool test_bit(int p) const noexcept
    {
        return bitset[word_index(p)] & (1ULL << bit_index(p));
    }

public:
    void insert(const T& value, int priority) noexcept
    {
        size++;
        buckets[priority].push_back(value);
        set_bit(priority);
    }

    inline bool empty() const noexcept
    {
        return size == 0;
    }

    int find_next_non_empty(int start) const noexcept
    {
        int w = word_index(start);
        uint64_t word = bitset[w];

        // Mask out bits below start
        word &= (~0ULL << bit_index(start));

        while (true)
        {
            if (word != 0)
                return w * WORD_SIZE + __builtin_ctzll(word);
            w++;
            if (w >= NUM_WORDS)
                return -1;
            word = bitset[w];
        }
    }

    std::vector<T> extract_bucket(int p) noexcept
    {
        // Move the entire bucket out
        std::vector<T> result = std::move(buckets[p]);

        clear_bit(p);
        size -= result.size();

        return result;
    }
};