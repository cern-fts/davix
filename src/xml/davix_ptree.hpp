/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Adrien Devresse <adrien.devresse@cern.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
*/

#ifndef DAVIX_PTREE_HPP
#define DAVIX_PTREE_HPP

#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>

namespace Davix {

template <class KeyType, class DataType, class PredEqualKey = std::equal_to<KeyType>, class PredEqualData = std::equal_to<DataType> >
class BasicPtree{
public:
    typedef BasicPtree<KeyType, DataType, PredEqualKey, PredEqualData> tree_type;
    typedef tree_type* ptr_type;
    typedef std::vector<tree_type> ChildrenList;
    typedef typename ChildrenList::const_iterator const_iterator;
    typedef typename ChildrenList::iterator iterator;


    BasicPtree() :
        _key(KeyType()),
        _data(DataType()),
        _children(ChildrenList()),
        _meta_data(NULL),
        _data_cmp(PredEqualData()),
        _key_cmp(PredEqualKey())
         {}
    BasicPtree(const KeyType & key, const DataType & data, const ChildrenList & l = ChildrenList(), void* meta = NULL) :
        _key(key), _data(data), _children(l), _meta_data(meta){}
    BasicPtree(const BasicPtree & orig) : _key(orig._key), _data(orig._data),  _children(orig._children), _meta_data(orig._meta_data) {}
    ~BasicPtree(){}

    inline const_iterator beginChildren() const{
        return _children.begin();
    }

    inline iterator beginChildren(){
        return _children.begin();
    }


    inline const_iterator endChildren() const{
        return _children.end();
    }

    inline iterator endChildren(){
        return _children.end();
    }

    inline void clearChildren(){
        _children.clear();
    }

    inline const KeyType  & getKey() const {
        return _key;
    }

    inline const DataType  & getData() const {
        return _data;
    }

    inline void* getMeta(){
        return _meta_data;
    }

    inline void setMeta(void* meta){
        _meta_data = meta;
    }

    inline BasicPtree & addChild(const BasicPtree & child){
        _children.push_back(child);
        return _children.back();
    }

    inline bool compareData(const BasicPtree & other) const{
        return ( _data_cmp(_data, other._data));
    }

    inline bool compareKey(const BasicPtree & other) const{
        return ( _key_cmp(_key, other._key));
    }

    inline int compareNode(const BasicPtree & other) const{
        return (compareKey(other) && compareData(other));
    }

    inline bool matchTree(const BasicPtree & rtree) const{
        if(compareNode(rtree) == false){
            return false;
        }

        for(const_iterator it_rtree = rtree.beginChildren(); it_rtree != rtree.endChildren(); ++it_rtree){
            bool found=false;
            for(const_iterator it = beginChildren(); it != endChildren(); ++it){
                if(it->matchTree(*it_rtree) ){
                    found = true;
                    break;
                }
            }
            if(!found){ // failed to find rtree node in main tree
                return false;
            }
        }
        std::cout << " end node " << rtree._data << std::endl;
        return true;
    }

    ///
    /// Match an order chain of node as a subtree
    /// return the chain matched
    inline std::vector<ptr_type> findChain(const std::vector<tree_type> & chain){
        std::vector<ptr_type> res;
        res.reserve(chain.size());
        _findChainRec(chain.begin(), chain.end(), res);
        return res;
    }

protected:
    // members
    KeyType _key;
    DataType _data;
    ChildrenList _children;
    void* _meta_data;
    // cmp
    PredEqualData _data_cmp;
    PredEqualKey _key_cmp;

    bool _findChainRec(typename std::vector<tree_type>::const_iterator begin,
                              typename std::vector<tree_type>::const_iterator end,
                              std::vector<tree_type::ptr_type> & res){
        if(begin == end)
            return true;

        if( compareNode(*begin)){
            res.push_back(this);
            begin++;
            for(iterator it = beginChildren(); it != endChildren(); ++it){
                if( it->_findChainRec(begin, end, res) == true){
                    return true;
                }
            }
        }
        return false;
    }
};


namespace Xml{

enum NodeType{
    ElementStart=0x01,
    CData=0x02,
    Attribute=0x03,
    Comment=0x04
};


typedef BasicPtree<NodeType, std::string> XmlPTree;


} // namespace Xml

} // namespace Davix

#endif // DAVIX_PTREE_HPP
