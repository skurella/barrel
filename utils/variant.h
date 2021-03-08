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

template<typename T, typename ...Ts>
struct head {
    using type = T;
};

template<typename T>
struct return_type;

template<class C, typename R, typename ...Args>
struct return_type<R(C::*)(Args...)> {
    using type = R;
};

template<class C, typename R, typename ...Args>
struct return_type<R(C::*)(Args...) const> {
    using type = R;
};

/**
 * A tagged union, i.e. an union which stores which union member is active.
 * Each type parameter is a possible union member.
 */
template <typename ...Ts>
struct variant {
private:
    int v;
    union_<Ts...> u;

public:
    template<typename T>
    variant(T value) :
        v(index_of<T, Ts...>::value),
        u(*reinterpret_cast<union_<Ts...>*>(&value)) {}

    variant<Ts...> &operator=(variant<Ts...>&) = default;
    variant<Ts...> &operator=(variant<Ts...> other) {
        this->v = other.v;
        this->u = other.u;
        return *this;
    }

    // Default-constructed variant is uninitialized. It is UB to use it without initializing first.
    variant() {}

    /**
     * Match on the variant.
     *
     * `visit` expects as many arguments as there are variant members.
     * Each argument must be a function accepting its respective member.
     * Each function must return the same type, which will be the return type of `visit`.
     *
     * The return type is inferred from the first function. (inspired by <https://functionalcpp.wordpress.com/2013/08/05/function-traits/>)
     */
    template<typename ...Fs>
    auto visit(Fs... fs) const -> typename return_type<decltype(&head<Fs...>::type::operator())>::type {
        return visit_union<typename return_type<decltype(&head<Fs...>::type::operator())>::type>(this->v, this->u, fs...);
    }
};

}
