/**
 * An untagged union.
 * Each type parameter is a possible union member.
 *
 * e.g. `myunion<int, bool, string>` is isomorphic to:
 * 
 * ```
 * union {
 *   int i;
 *   bool b;
 *   string s;
 * }
 * ```
 */
template <typename ...Ts>
struct myunion;

template <typename T, typename ...Ts>
struct myunion<T, Ts...> {
    union {
        T here;
        myunion<Ts...> there;
    } value;
};

template <>
struct myunion<> {};
 
/**
 * A tagged union, i.e. an union which stores which union member is active.
 * Each type parameter is a possible union member.
 */
template <typename ...Ts>
struct myvariant {
    int v;
    myunion<Ts...> u;

    template<typename R>
    R visit(std::function<R(Ts)>... fs) const {
        return visit_union(this->v, this->u, fs...);
    }
};

template<typename R, typename T>
R visit_union(int, const myunion<T> &u, std::function<R(T)> f) {
    return f(u.value.here);
}

template<typename R, typename T, typename ...Ts>
R visit_union(int index, const myunion<T, Ts...> &u, std::function<R(T)> f, std::function<R(Ts)>... fs) {
    if(index == 0) {
        return f(u.value.here);
    } else {
        return visit_union(index - 1, u.value.there, fs...);
    }
}
