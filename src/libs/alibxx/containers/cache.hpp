#pragma once

#include <functional>
#include <algorithm>
#include <map>
#include <limits>
#include <utility>
#include <mutex>
#include <memory>


namespace Davix {

///
/// Thread Safe Cache container
///
template <class Key, class Value, class CompareT = std::less<Key> >
class Cache {
    typedef std::map<Key, std::shared_ptr<Value> > Map;
    typedef std::shared_ptr<Value> shrPtr_type;

public:
    Cache(size_t max_size = std::numeric_limits<size_t>::max()) :
        cmp(CompareT()), map(cmp), _max_size(max_size), _m(){}

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
        std::lock_guard<std::mutex> l(_m);
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
        std::lock_guard<std::mutex> l(_m);
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
        std::lock_guard<std::mutex> l(_m);
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
        std::lock_guard<std::mutex> l(_m);
        return map.size();
    }

    ///
    /// \brief remove an object from this cache and return its reference
    /// \param key
    /// \return
    ///
    shrPtr_type take( const Key & key){
        std::lock_guard<std::mutex> l(_m);
        typename Map::iterator it = map.find( key);
        if(it == map.end()) return shrPtr_type();

        shrPtr_type ret = it->second;
        map.erase(it);
        return ret;
    }

    ///
    /// \brief erase an object from the cache
    /// \param key
    /// \return
    ///
    bool erase( const Key & key){
        std::lock_guard<std::mutex> l(_m);
        return ( map.erase(key) != 0);
    }

    void clear(){
        std::lock_guard<std::mutex> l(_m);
        map.clear();
    }


protected:
    CompareT cmp;
    Map map;
    size_t _max_size;
    mutable std::mutex _m;

    void auto_clean(){
        if(map.size() == _max_size){
            map.clear();
        }
    }
};




}
