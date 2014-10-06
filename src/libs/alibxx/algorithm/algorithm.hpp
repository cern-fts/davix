#ifndef ALGORITHM_HPP
#define ALGORITHM_HPP



namespace A_LIB_NAMESPACE{

template<typename _Iterator1, typename _Iterator2, typename _Tvalue>
_Iterator2 match_array(_Iterator1 begin_match, _Iterator1 end_match,
                       _Iterator2 begin_resu, _Iterator2 end_resu, _Tvalue val){
    do{
        if(*begin_match == val)
            return begin_resu;

        begin_match++;
        begin_resu++;
    }while( begin_match < end_match && begin_resu < end_resu);
    return end_resu;
}

}

#endif // ALGORITHM_HPP
