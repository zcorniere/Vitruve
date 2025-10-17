#pragma once

#include "Engine/Containers/Tuple.hxx"

template <typename TKey, typename TValue, typename THasher = std::hash<TKey>, float FLoadFactor = 0.75f,
          typename TSizeType = uint32, unsigned MinimalSize = 8>
requires CHashable<TKey>
class TMap
{
private:
    using Bucket = TArray<TPair<TKey, TValue>>;

    class Iterator
    {
    public:
        Iterator(TMap& InMap, TSizeType Index)
            : StartingSize(InMap.NumElements)
            , Map(InMap)
            , Index(Index)
            , CurrentBucket(nullptr)
            , CurrentBucketIndex(0)

        {
            if (Index < Map.Buckets.Size())
            {
                CurrentBucket = &Map.Buckets[Index];
                if (CurrentBucket->Size() == 0)
                {
                    AdvanceBucket();
                }
            }
        }

        TPair<TKey, TValue>& operator*()
        {
            CheckIsIteratorValid();

            check(CurrentBucket != nullptr);
            return CurrentBucket->operator[](CurrentBucketIndex);
        }
        TPair<TKey, TValue>* operator->()
        {
            CheckIsIteratorValid();

            check(CurrentBucket != nullptr);
            return &CurrentBucket->operator[](CurrentBucketIndex);
        }

        Iterator& operator++()
        {
            CheckIsIteratorValid();

            AdvanceBucketIndex();
            return *this;
        }
        Iterator operator++(int)
        {
            CheckIsIteratorValid();
            AdvanceBucketIndex();
            return *this;
        }

        bool operator==(const Iterator& Other) const
        {
            CheckIsIteratorValid();
            checkSlow(StartingSize == Other.StartingSize);
            checkSlow(&Map == &Other.Map);
            return Index == Other.Index && CurrentBucketIndex == Other.CurrentBucketIndex &&
                   CurrentBucket == Other.CurrentBucket;
        }

    private:
        void AdvanceBucketIndex()
        {
            if (CurrentBucket == nullptr)
            {
                AdvanceBucket();
            }
            else
            {
                CurrentBucketIndex++;
            }
            if (CurrentBucketIndex >= CurrentBucket->Size())
            {
                AdvanceBucket();
            }
        }
        void AdvanceBucket()
        {
            Index++;
            CurrentBucketIndex = 0;
            if (Index < Map.Buckets.Size())
            {
                CurrentBucket = &Map.Buckets[Index];
                if (CurrentBucket->Size() == 0)
                {
                    AdvanceBucket();
                }
            }
            else
            {
                CurrentBucket = nullptr;
            }
        }
        void CheckIsIteratorValid() const
        {
            checkMsg(StartingSize == Map.NumElements, "The map has been modified while iterating");
        }

    private:
        const TSizeType StartingSize = 0;

        TMap& Map;
        TSizeType Index;

        Bucket* CurrentBucket = nullptr;
        TSizeType CurrentBucketIndex = 0;
    };

    class ConstIterator
    {
    public:
        ConstIterator(const TMap& InMap, TSizeType Index)
            : StartingSize(InMap.NumElements)
            , Map(InMap)
            , Index(Index)
            , CurrentBucket(nullptr)
            , CurrentBucketIndex(0)

        {
            if (Index < Map.Buckets.Size())
            {
                CurrentBucket = &Map.Buckets[Index];
                if (CurrentBucket->Size() == 0)
                {
                    AdvanceBucket();
                }
            }
        }

        const TPair<TKey, TValue>& operator*()
        {
            CheckIsIteratorValid();

            check(CurrentBucket != nullptr);
            return CurrentBucket->operator[](CurrentBucketIndex);
        }
        const TPair<TKey, TValue>* operator->()
        {
            CheckIsIteratorValid();

            check(CurrentBucket != nullptr);
            return CurrentBucket->operator[](CurrentBucketIndex);
        }

        ConstIterator& operator++()
        {
            CheckIsIteratorValid();

            AdvanceBucketIndex();
            return *this;
        }
        ConstIterator operator++(int)
        {
            CheckIsIteratorValid();
            AdvanceBucketIndex();
            return *this;
        }

        bool operator==(const ConstIterator& Other) const
        {
            CheckIsIteratorValid();
            checkSlow(StartingSize == Other.StartingSize);
            checkSlow(&Map == &Other.Map);
            return Index == Other.Index && CurrentBucketIndex == Other.CurrentBucketIndex &&
                   CurrentBucket == Other.CurrentBucket;
        }

    private:
        void AdvanceBucketIndex()
        {
            if (CurrentBucket == nullptr)
            {
                AdvanceBucket();
            }
            else
            {
                CurrentBucketIndex++;
            }
            if (CurrentBucketIndex >= CurrentBucket->Size())
            {
                AdvanceBucket();
            }
        }
        void AdvanceBucket()
        {
            Index++;
            CurrentBucketIndex = 0;
            if (Index < Map.Buckets.Size())
            {
                CurrentBucket = &Map.Buckets[Index];
                if (CurrentBucket->Size() == 0)
                {
                    AdvanceBucket();
                }
            }
            else
            {
                CurrentBucket = nullptr;
            }
        }
        void CheckIsIteratorValid() const
        {
            checkMsg(StartingSize == Map.NumElements, "The map has been modified while iterating");
        }

    private:
        const TSizeType StartingSize = 0;

        const TMap& Map;
        TSizeType Index;

        const Bucket* CurrentBucket = nullptr;
        TSizeType CurrentBucketIndex = 0;
    };

public:
    FORCEINLINE TMap()
    {
        Rehash();
    }

    FORCEINLINE TMap(const std::initializer_list<TPair<TKey, TValue>>& InitList)
    {
        Rehash(InitList.size() > 0 ? InitList.size() : MinimalSize);

        for (const TPair<TKey, TValue>& Pair: InitList)
        {
            Insert(Pair.template Get<0>(), Pair.template Get<1>());
        }
    }

    FORCEINLINE Iterator begin()
    {
        return Iterator(*this, 0);
    }
    FORCEINLINE Iterator end()
    {
        return Iterator(*this, Buckets.Size());
    }

    FORCEINLINE ConstIterator begin() const
    {
        return ConstIterator(*this, 0);
    }
    FORCEINLINE ConstIterator end() const
    {
        return ConstIterator(*this, Buckets.Size());
    }
    FORCEINLINE ConstIterator cbegin() const
    {
        return ConstIterator(*this, 0);
    }
    FORCEINLINE ConstIterator cend() const
    {
        return ConstIterator(*this, Buckets.Size());
    }

    /// @brief Emplace a new element in the map
    /// @param Key The key of the element
    template <typename... ArgsTypes>
    FORCEINLINE TValue& Emplace(const TKey& Key, ArgsTypes&&... Args)
    {
        return Insert(Key, TValue(std::forward<ArgsTypes>(Args)...));
    }

    /// @brief Insert a new element in the map
    /// @param Key The key of the element
    /// @return The reference to the of the element
    TValue& Insert(const TKey& Key, const TValue& Value)
    {
        TValue* const FoundValue = Find(Key);
        if (FoundValue != nullptr)
        {
            *FoundValue = Value;
            return *FoundValue;
        }

        if (NumElements >= Buckets.Size() * FLoadFactor)
        {
            Rehash(Buckets.Size() * 2);    // Rehash to double the size
        }

        const TSizeType HashValue = Hash(Key);
        const TSizeType BucketIndex = HashValue % Buckets.Size();
        Bucket& CurrentBucket = Buckets[BucketIndex];

        CurrentBucket.Emplace(Key, Value);
        NumElements++;
        return CurrentBucket.Back().template Get<1>();
    }

    /// @brief Insert a new element in the map
    /// @param Key The key of the element (will be moved)
    /// @return The reference to the of the element
    TValue& Insert(const TKey& Key, TValue&& Value)
    {
        TValue* const FoundValue = Find(Key);
        if (FoundValue != nullptr)
        {
            *FoundValue = std::move(Value);
            return *FoundValue;
        }

        if (NumElements >= Buckets.Size() * FLoadFactor)
        {
            Rehash(Buckets.Size() * 2);    // Rehash to double the size
        }

        const TSizeType HashValue = Hash(Key);
        const TSizeType BucketIndex = HashValue % Buckets.Size();
        Bucket& CurrentBucket = Buckets[BucketIndex];

        CurrentBucket.Emplace(Key, std::move(Value));
        NumElements++;
        return CurrentBucket.Back().template Get<1>();
    }

    /// @brief Remove an element from the map
    /// @param Key The key of the element to remove
    /// @return true if the element was removed
    bool Remove(const TKey& Key, TPair<TKey, TValue>* RemovedValue = nullptr)
    {
        // Pick the bucket
        const TSizeType HashValue = Hash(Key);
        const TSizeType BucketIndex = HashValue % Buckets.Size();
        Bucket& CurrentBucket = Buckets[BucketIndex];

        // Find the element in the bucket, and remove it
        for (TSizeType i = 0; i < CurrentBucket.Size(); i++)
        {
            const TPair<TKey, TValue>& Pair = CurrentBucket[i];
            const TSizeType PairHash = Hash(Pair.template Get<0>());
            if (PairHash == HashValue && Pair.template Get<0>() == Key)
            {
                CurrentBucket.RemoveAt(i, RemovedValue);
                NumElements--;
                return true;
            }
        }
        return false;
    }

    /// @brief Get the number of elements in the map
    FORCEINLINE TSizeType Size() const
    {
        return NumElements;
    }

    /// @brief Check if the map is empty
    FORCEINLINE bool IsEmpty() const
    {
        return Size() == 0;
    }

    /// @brief Get the number of buckets in the map
    FORCEINLINE TSizeType BucketCount() const
    {
        return Buckets.Size();
    }

    TValue& FindOrAdd(const TKey& Key)
    requires(std::is_default_constructible<TValue>::value)
    {
        TValue* Value = Find(Key);
        if (Value == nullptr)
        {
            return Emplace(Key);
        }
        return *Value;
    }

    /// @brief Find an element in the map
    /// @param Key The key of the element to find
    /// @return The value of the element if found, nullptr otherwise
    const TValue* Find(const TKey& Key) const
    {
        if (Buckets.Size() == 0)
        {
            return nullptr;
        }

        const TSizeType HashValue = Hash(Key);
        const TSizeType BucketIndex = HashValue % Buckets.Size();
        const Bucket& CurrentBucket = Buckets[BucketIndex];

        for (const TPair<TKey, TValue>& Pair: CurrentBucket)
        {
            const TSizeType PairHash = Hash(Pair.template Get<0>());
            if (PairHash == HashValue)
            {
                return &Pair.template Get<1>();
            }
        }
        return nullptr;
    }

    /// @brief Find an element in the map
    /// @param Key The key of the element to find
    /// @return The value of the element if found, nullptr otherwise
    TValue* Find(const TKey& Key)
    {
        if (Buckets.Size() == 0)
        {
            return nullptr;
        }

        const TSizeType HashValue = Hash(Key);
        const TSizeType BucketIndex = HashValue % Buckets.Size();
        Bucket& CurrentBucket = Buckets[BucketIndex];

        for (TPair<TKey, TValue>& Pair: CurrentBucket)
        {
            const TSizeType PairHash = Hash(Pair.template Get<0>());
            if (PairHash == HashValue && Pair.template Get<0>() == Key)
            {
                return &Pair.template Get<1>();
            }
        }
        return nullptr;
    }

    /// @brief Check if an element is in the map
    /// @param Key The key of the element to find
    /// @return true if the element is in the map
    FORCEINLINE bool Contains(const TKey& Key) const
    {
        return Find(Key) != nullptr;
    }

    void Rehash(const TSizeType NewCount = MinimalSize)
    {
        TArray<Bucket, 16u> NewBuckets(NewCount);
        for (const Bucket& CurrentBucket: Buckets)
        {
            for (const TPair<TKey, TValue>& Pair: CurrentBucket)
            {
                const TSizeType HashValue = Hash(Pair.template Get<0>());
                const TSizeType BucketIndex = HashValue % NewBuckets.Size();
                NewBuckets[BucketIndex].Add(Pair);
            }
        }
        Buckets = std::move(NewBuckets);
    }

    /// @brief Remove all elements from the map
    /// @note The map is never truly empty, it is rehashed to a minimum size
    FORCEINLINE void Clear()
    {
        Buckets.Clear();
        NumElements = 0;

        // The map cannot really be empty, so we rehash to a minimum size
        Rehash();
    }

    // operator[] cannot fail
    FORCEINLINE const TValue& operator[](const TKey& Key) const
    {
        const TValue* const Value = Find(Key);
        check(Value != nullptr);
        return *Value;
    }

    FORCEINLINE TValue& operator[](const TKey& Key)
    {
        TValue* const Value = Find(Key);
        check(Value != nullptr);
        return *Value;
    }

    FORCEINLINE bool operator==(const TMap<TKey, TValue, THasher, FLoadFactor, TSizeType, MinimalSize>& Other) const
    requires std::equality_comparable<TKey> && std::equality_comparable<TValue>
    {
        if (NumElements != Other.NumElements)
        {
            return false;
        }

        // For each element in this map, check if it exists in the other map with the same value
        for (const auto& pair: *this)
        {
            const TValue* otherValue = Other.Find(pair.template Get<0>());
            if (!otherValue || *otherValue != pair.template Get<1>())
            {
                return false;
            }
        }

        return true;
    }

private:
    FORCEINLINE TSizeType Hash(const TKey& Key) const
    {
        return THasher{}(Key);
    }

    FORCEINLINE TSizeType GetBucketIndex(const TKey& Key) const
    {
        return Hash(Key) % Buckets.Size();
    }

    TArray<Bucket, 16u> Buckets;
    TSizeType NumElements = 0;

    friend class Iterator;
};

template <typename TKey, typename TValue>
std::ostream& operator<<(std::ostream& os, const TMap<TKey, TValue>& m)
{
    os << std::format("{}", m);
    return os;
}

template <typename TKey, typename TValue>
struct std::formatter<TMap<TKey, TValue>> : std::formatter<TKey>
{

    template <class FormatContext>
    auto format(const TMap<TKey, TValue>& Value, FormatContext& ctx) const
    {
        auto&& out = ctx.out();
        format_to(out, "[{}]", Value.Size());
        return out;
    }
};
