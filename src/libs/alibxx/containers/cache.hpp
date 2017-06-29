#pragma once

#include <functional>
#include <algorithm>
#include <map>
#include <limits>
#include <utility>
#include <mutex>


namespace A_LIB_NAMESPACE{

///
/// Thread Safe Cache container. Thread safe in the sense that it must be locked from outside
///
template <class Key, class Value, class CompareT = std::less<Key> >

// We inherit from mutex to be able to lock the cache altogether also from outside
class Cache : public std::mutex {
    typedef std::map<Key, std::shared_ptr<Value> > Map;
    typedef std::shared_ptr<Value> shrPtr_type;

public:
    Cache(size_t max_size = std::numeric_limits<size_t>::max()) :
        cmp(CompareT()), map(cmp), _max_size(max_size) {}

    ~Cache(){}

    shrPtr_type insert(const Key & key, Value* value){
        return insert(key, shrPtr_type(value));
    }

    ///
    /// \brief insert a new value into the cache, if an already existing one is mapped to this case, the old value is replaced
    /// \param key
    /// \param value
    /// \return
    ///
    shrPtr_type insert(const Key & key, const shrPtr_type & value){
        auto_clean();
        std::pair< typename Map::iterator, bool > stored = map.insert(std::pair<Key,shrPtr_type> (key, value) );
        if(stored.second)
            return (*stored.first).second;
        (*stored.first).second = value;
        return value;
    }

    ///
    /// \brief find a cached value, returned a NULL shared_pointer if not found
    /// \param key
    /// \return
    ///
    shrPtr_type find(const Key & key){
        typename Map::iterator it = map.find( key);
        if(it == map.end())
            return shrPtr_type();
        return it->second;
    }

    ///
    /// \brief return the key that is immediatelly after given key
    /// \param key
    /// \return the next key
    ///
    const Key upper_bound(const Key & key) {
        typename Map::iterator it = map.upper_bound( key);
        if(it == map.end())
            return Key();
        return it->first;
    }

    ///
    /// \brief getSize return the number of object sotred in the cache
    /// \return
    ///
    size_t getSize() const{
        return map.size();
    }

    ///
    /// \brief remove an object from this cache and return its reference
    /// \param key
    /// \return
    ///
    shrPtr_type take( const Key & key){
        typename Map::iterator it = map.find( key);
        if(it == map.end())
            return shrPtr_type();
        map.erase(key);
        return it->second;
    }

    ///
    /// \brief erase an object from the cache
    /// \param key
    /// \return
    ///
    bool erase( const Key & key){
        return ( map.erase(key) != 0);
    }

    void clear(){
        map.clear();
    }


protected:
    CompareT cmp;
    Map map;
    size_t _max_size;

    void auto_clean(){
        if(map.size() >= _max_size){
            map.clear();
        }
    }
};




}
