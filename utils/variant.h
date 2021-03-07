namespace utils {

/**
 * An untagged union.
 * Each type parameter is a possible union member.
 *
 * e.g. `union_<int, bool, string>` is isomorphic to:
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
struct union_;

template <typename T, typename ...Ts>
struct union_<T, Ts...> {
    union {
        T here;
        union_<Ts...> there;
    } value;
};

template <>
struct union_<> {};

template<typename R, typename T, typename F>
R visit_union(int, const union_<T> &u, F f) {
    return f(u.value.here);
}

template<typename R, typename T, typename ...Ts, typename F, typename ...Fs>
R visit_union(int index, const union_<T, Ts...> &u, F f, Fs... fs) {
    if(index == 0) {
        return f(u.value.here);
    } else {
        return visit_union<R>(index - 1, u.value.there, fs...);
    }
}

template<typename Needle, typename ...Haystack>
struct index_of;

template<typename T, typename ...Ts>
struct index_of<T, T, Ts...> {
    static constexpr int value = 0;
};

template<typename T, typename X, typename ...Ts>
struct index_of<T, X, Ts...> {
    static constexpr int value = index_of<T, Ts...>::value + 1;
};

/**
 * A tagged union, i.e. an union which stores which union member is active.
 * Each type parameter is a possible union member.
 */
template <typename ...Ts>
struct variant {
    int v;
    union_<Ts...> u;

    template<typename T>
    variant(T value) :
        v(index_of<T, Ts...>::value),
        u(*reinterpret_cast<union_<Ts...>*>(&value)) {}

    template<typename R, typename ...Fs>
    R visit(Fs... fs) const {
        return visit_union<R>(this->v, this->u, fs...);
    }
};

}
