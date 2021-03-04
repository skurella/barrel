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

template<typename R, typename T, typename F>
R visit_union(int, const myunion<T> &u, F f) {
    return f(u.value.here);
}

template<typename R, typename T, typename ...Ts, typename F, typename ...Fs>
R visit_union(int index, const myunion<T, Ts...> &u, F f, Fs... fs) {
    if(index == 0) {
        return f(u.value.here);
    } else {
        return visit_union<R>(index - 1, u.value.there, fs...);
    }
}

/**
 * A tagged union, i.e. an union which stores which union member is active.
 * Each type parameter is a possible union member.
 */
template <typename ...Ts>
struct myvariant {
    int v;
    myunion<Ts...> u;

    template<typename R, typename ...Fs>
    R visit(Fs... fs) const {
        return visit_union<R>(this->v, this->u, fs...);
    }
};
