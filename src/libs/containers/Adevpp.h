#ifndef ADEVPP_CACHE_H
#define ADEVPP_CACHE_H

#include <functional>
#include <algorithm>
#include <map>
#include <utility>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>


namespace Adevpp{

template <class Key, class Value, class CompareT = std::less<Key> >
class Cache {
    typedef std::map<Key, boost::shared_ptr<T> > Map;
    typedef boost::shared_ptr<T> shrPtr_type;

public:
    Cache() : cmp(CompareT()), map(cmp){}

    shrPtr_type insert(const Key & key, T* value){
        return insert(key, shrPtr_type(value));
    }

    shrPtr_type insert(const Key & key, const shrPtr_type & value){
        boost::lock_guard<boost::mutex> l(m);
        Map::iterator it = map.find( key);
        if(it == Map::end())
            return map.insert(std::pair<Key,shrPtr_type> (key, value) ).second;
        it->second = value;
        return  value;
    }

    shrPtr_type find(const Key & key){
        boost::lock_guard<boost::mutex> l(m);
        Map::iterator it = map.find( key);
        if(it == Map::end())
            return shrPtr_type();
        return *it;
    }

    size_t getSize() const{
        boost::lock_guard<boost::mutex> l(m);
        return map.size();
    }

    shrPtr_type take( const Key & key){
        boost::lock_guard<boost::mutex> l(m);
        Map::iterator it = map.find( key);
        if(it == Map::end())
            return shrPtr_type();
        map.erase(key);
        return *it;
    }


    bool erase( const Key & key){
        boost::lock_guard<boost::mutex> l(m);
        return ( map.erase(key) != 0);
    }

    void clear(){
        boost::lock_guard<boost::mutex> l(m);
        map.clear();
    }


private:
    CompareT cmp;
    Map map;
    boost::mutex m;
};




}

#endif // ADEVPP_H
