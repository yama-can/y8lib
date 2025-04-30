#include <y8lib/internal/random.hpp>
#include <cassert>

template <typename T, T (*op)(T, T), T (*e)(), typename RandomEngine = mwc256xxa64>
class Treap
{
	struct node
	{
		T value, raw_value;
		size_t subtree_size;
		RandomEngine::result_type priority;
		node *left, *right, *parent;
	};

	node *root;

	RandomEngine random_engine;

	inline void find_node(node *&now, size_t pos) const
	{
		while (true)
		{
			if (!now->left)
			{
				if (pos == 0)
				{
					break;
				}
				pos--;
				now = now->right;
				continue;
			}
			if (pos < now->left->subtree_size)
			{
				now = now->left;
			}
			else if (pos > now->left->subtree_size)
			{
				pos -= now->left->subtree_size + 1;
				now = now->right;
			}
			else
			{
				return;
			}
		}
		assert(now && pos == 0);
	}

public:
	using value_type = T;
	using size_type = size_t;

	class iterator
	{
		Treap *treap;
		node *now;
		iterator(Treap *treap, node *now) : treap{treap}, now(now)
		{
		}
		friend class Treap;

	public:
		iterator() = delete;
		iterator(const iterator &other) = default;
		iterator &operator=(const iterator &other) = default;
		iterator(iterator &&other) = default;
		iterator &operator=(iterator &&other) = default;
		~iterator() = default;
		iterator &operator++()
		{
			if (now->right)
			{
				now = now->right;
				while (now->left)
				{
					now = now->left;
				}
			}
			else
			{
				while (now->parent && now->parent->right == now)
				{
					now = now->parent;
				}
				now = now->parent;
			}
			return *this;
		}
		iterator operator++(int)
		{
			iterator tmp = *this;
			++(*this);
			return tmp;
		}
		iterator &operator--()
		{
			if (now->left)
			{
				now = now->left;
				while (now->right)
				{
					now = now->right;
				}
			}
			else
			{
				while (now->parent && now->parent->left == now)
				{
					now = now->parent;
				}
				now = now->parent;
			}
			return *this;
		}
		iterator operator--(int)
		{
			iterator tmp = *this;
			--(*this);
			return tmp;
		}
		T &operator*()
		{
			return now->raw_value;
		}
		const T &operator*() const
		{
			return now->raw_value;
		}
		T *operator->()
		{
			return &now->raw_value;
		}
		const T *operator->() const
		{
			return &now->raw_value;
		}
		bool operator==(const iterator &other) const
		{
			return now == other.now;
		}
		bool operator!=(const iterator &other) const
		{
			return now != other.now;
		}
		size_t indexof()
		{
			if(!now)
				return treap->size();
			size_t pos = 0;
			node *now = this->now;
			while (now)
			{
				if (now->parent && now->parent->left == now)
				{
					pos += now->parent->right ? now->parent->right->subtree_size + 1 : 1;
				}
				now = now->parent;
			}
			return pos;
		}
		long long operator-(iterator other)
		{
			assert(treap == other.treap);
			return indexof() - other.indexof();
		}
	};

	Treap() : root(nullptr)
	{
	}

	void insert(size_t pos, T value)
	{
		if (!root)
		{
			root = new node{value, value, 1, random_engine(), nullptr, nullptr, nullptr};
			return;
		}
		node *now = root;
		if (pos != size())
		{
			find_node(now, pos);
			if (now->left)
			{
				now = now->left;
				while (now->right)
				{
					now = now->right;
				}
				now->right = new node{value, value, 1, random_engine(), nullptr, nullptr, now};
				now = now->right;
			}
			else
			{
				now->left = new node{value, value, 1, random_engine(), nullptr, nullptr, now};
				now = now->left;
			}
		}
		else
		{
			while (now->right)
			{
				now = now->right;
			}
			now->right = new node{value, value, 1, random_engine(), nullptr, nullptr, now};
			now = now->right;
		}
		assert(now);
		while (now)
		{
			if (now->left && now->priority > now->left->priority)
			{
				node *left = now->left;
				if (!now->parent)
					root = left;
				else if (now->parent->left == now)
					now->parent->left = left;
				else
					now->parent->right = left;
				left->parent = now->parent;
				now->parent = left;
				now->left = left->right;
				if (left->right)
					left->right->parent = now;
				left->right = now;
				now->subtree_size = 1 + (now->left ? now->left->subtree_size : 0) + (now->right ? now->right->subtree_size : 0);
				now->value = op(op(now->left ? now->left->value : e(), now->raw_value), now->right ? now->right->value : e());
				now = left;
			}
			else if (now->right && now->priority > now->right->priority)
			{
				node *right = now->right;
				if (!now->parent)
					root = right;
				else if (now->parent->left == now)
					now->parent->left = right;
				else
					now->parent->right = right;
				right->parent = now->parent;
				now->parent = right;
				now->right = right->left;
				if (right->left)
					right->left->parent = now;
				right->left = now;
				now->subtree_size = 1 + (now->left ? now->left->subtree_size : 0) + (now->right ? now->right->subtree_size : 0);
				now->value = op(op(now->left ? now->left->value : e(), now->raw_value), now->right ? now->right->value : e());
				now = right;
			}
			now->subtree_size = 1 + (now->left ? now->left->subtree_size : 0) + (now->right ? now->right->subtree_size : 0);
			now->value = op(op(now->left ? now->left->value : e(), now->raw_value), now->right ? now->right->value : e());
			now = now->parent;
		}
	}

	void erase(size_t pos)
	{
		erase(find(pos));
	}

	void erase(iterator it)
	{
		node *now = it.now;
		while (now->subtree_size > 1)
		{
			if (now->left)
			{
				std::swap(now->left->raw_value, now->raw_value);
				now = now->left;
			}
			else
			{
				std::swap(now->right->raw_value, now->raw_value);
				now = now->right;
			}
		}
		if (now == root)
		{
			delete now;
			root = nullptr;
			return;
		}
		if (now->parent->left == now)
			now->parent->left = nullptr;
		else
			now->parent->right = nullptr;
		node *top = now->parent;
		delete now;
		while (top)
		{
			top->subtree_size = 1 + (top->left ? top->left->subtree_size : 0) + (top->right ? top->right->subtree_size : 0);
			top->value = op(op(top->left ? top->left->value : e(), top->raw_value), top->right ? top->right->value : e());
			top = top->parent;
		}
	}

	T prod(size_t l, size_t r) const
	{
		assert(0 <= l && l <= r && r <= size());
		T val = e();
		while (l != r)
		{
			node *now = root;
			find_node(now, l);
			if (now->subtree_size == 1 || l + now->subtree_size > r)
			{
				val = op(val, now->raw_value);
				l++;
			}
			else
			{
				while (now->parent && now->parent->subtree_size + l <= r)
				{
					now = now->parent;
				}
				val = op(val, now->value);
				l += now->subtree_size;
			}
		}
		return val;
	}

	iterator begin()
	{
		node *now = root;
		while (now && now->left)
		{
			now = now->left;
		}
		return iterator(this, now);
	}

	iterator end()
	{
		return iterator(this, nullptr);
	}

	iterator find(size_t pos)
	{
		node *now = root;
		find_node(now, pos);
		return iterator(this, now);
	}

	size_t size() const
	{
		return root ? root->subtree_size : 0;
	}
};
