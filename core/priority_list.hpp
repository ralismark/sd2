/* -*- cpp.doxygen -*- */
#pragma once

#include <list>
#include <algorithm>
#include <functional>

/**
 * \class priority_list
 * \brief list always sorted based on a predicate
 *
 * Similar to a priority_queue, this guarantees that elements in the list are
 * sorted. However, the iterators to elements are not invalidated when
 * modifying the container, allowing references to elements (and modifying
 * them).
 */
template <typename T, typename Compare = std::less<T>>
class priority_list
	: private std::list<T>
{
private: // internal statics

	using c = std::list<T>;

public: // statics

	using container_type = c;
	using value_compare = std::decay_t<Compare>;

	using typename c::value_type;
	using typename c::size_type;
	using typename c::reference;
	using typename c::const_reference;
	using typename c::iterator;
	using typename c::const_iterator;
	using typename c::reverse_iterator;
	using typename c::const_reverse_iterator;

private: // variables

	value_compare cmp;

private: // internal methods

	auto gt_compare()
	{
		return [&](const auto& lhs, const auto& rhs) {
				return cmp(lhs, rhs);
			};
	}

	void sort()
	{
		c::sort(this->gt_compare());
	}

	const_iterator insertion_point(const value_type& value)
	{
		return std::upper_bound(this->cbegin(), this->cend(), value, this->gt_compare());
	}

	const_iterator sort_retain_end()
	{
		const_iterator last = c::end();
		--last;

		this->sort();
		return last;
	}

public: // methods

	// member functions {{{

	explicit priority_list(const value_compare& compare = value_compare(),
	                       container_type&& cont = container_type())
		: c(cont), cmp(std::move(compare))
	{
		this->sort();
	}

	priority_list(const value_compare& compare, const container_type& cont)
		: c(cont), cmp(compare)
	{
		this->sort();
	}

	priority_list(container_type&& cont)
		: priority_list(value_compare(), std::move(cont))
	{
		this->sort();
	}

	priority_list(const container_type& cont)
		: priority_list(value_compare(), cont)
	{
		this->sort();
	}

	using c::get_allocator;

	// }}}
	// capacity {{{

	using c::empty;
	using c::size;
	using c::max_size;

	// }}}
	// element access {{{

	const_reference top() const
	{
		return c::front();
	}

	// }}}
	// iterators {{{

	using c::cbegin;
	using c::cend;
	using c::crbegin;
	using c::crend;

	const_iterator begin() const
	{
		return this->cbegin();
	}

	const_iterator end() const
	{
		return this->cend();
	}

	const_reverse_iterator rbegin() const
	{
		return this->crbegin();
	}

	const_reverse_iterator rend() const
	{
		return this->crend();
	}

	// }}}
	// modifiers {{{

	const_iterator push(const value_type& value)
	{
		return c::insert(this->insertion_point(value), value);
	}

	const_iterator push(value_type&& value)
	{
		return c::insert(this->insertion_point(value), std::move(value));
	}

	void pop()
	{
		c::pop_front();
	}

	template <typename... Ts>
	const_iterator emplace(Ts&&... args)
	{
		c::emplace_back(std::forward<Ts>(args)...);
		return this->sort_retain_end();
	}

	const_iterator erase(const_iterator pos)
	{
		return c::erase(pos);
	}

	const_iterator erase(const_iterator first, const_iterator last)
	{
		return c::erase(first, last);
	}

	void swap(priority_list& other)
	{
		using std::swap;

		// swap underlying
		swap(static_cast<c&>(*this), static_cast<c&>(other));
		swap(cmp, other.cmp);
	}

	// }}}
	// conversions {{{

	operator const container_type&() const&
	{
		return static_cast<const c&>(*this);
	}

	operator container_type() const&&
	{
		return static_cast<const c&>(*this);
	}

	// }}}

};
