#ifndef DAVIX_PTREE_HPP
#define DAVIX_PTREE_HPP

#include <deque>
#include <algorithm>
#include <functional>
#include <iostream>

namespace Davix {

template <class KeyType, class DataType, class PredEqualKey = std::equal_to<KeyType>, class PredEqualData = std::equal_to<DataType> >
class BasicPtree{
public:
    typedef BasicPtree<KeyType, DataType, PredEqualKey, PredEqualData> tree_type;
    typedef typename std::deque<tree_type > ChildrenList;
    typedef typename ChildrenList::const_iterator const_iterator;

    BasicPtree(){}
    BasicPtree(const KeyType & key, const DataType & data, const ChildrenList & l = ChildrenList()) :
        _key(key), _data(data), _children(l){}
    BasicPtree(const BasicPtree & orig) : _key(orig._key), _data(orig._data), _children(orig._children) {}
    ~BasicPtree(){}

    inline const_iterator beginChildren() const{
        return _children.begin();
    }

    inline const_iterator endChildren() const{
        return _children.end();
    }

    inline KeyType  & getKey() const {
        return _key;
    }

    inline DataType  & getData() const {
        return _data;
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


protected:
    // members
    KeyType _key;
    DataType _data;
    ChildrenList _children;
    // cmp
    PredEqualData _data_cmp;
    PredEqualKey _key_cmp;


};


namespace Xml{

enum NodeType{
    ElementStart=0x01,
    ElementEnd=0x02,
    CData=0x03,
    Attribute=0x04,
    Comment=0x05
};


typedef BasicPtree<NodeType, std::string> XmlPTree;


} // namespace Xml

} // namespace Davix

#endif // DAVIX_PTREE_HPP
