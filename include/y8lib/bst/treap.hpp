#include <y8lib/internal/random.hpp>
#include <cassert>

namespace y8lib
{

	template <typename T, T (*op)(T, T), T (*e)(), typename RandomEngine = internal::mwc256xxa64>
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

		void reheap(node *&now)
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

		void merge_right(node *root, node *&other)
		{
			if (!other)
			{
				return;
			}
			node *right = root->right;
			if (!right)
			{
				root->right = other;
				other->parent = root;
				other = nullptr;
				node *now = root->right;
				while (now)
				{
					reheap(now);
				}
				return;
			}
			root->right = other;
			other->parent = root;
			node *now = root;
			while (now->right)
			{
				now = now->right;
			}
			now->right = right;
			right->parent = now;
			now = now->right;
			while (now)
			{
				reheap(now);
			}
			other = nullptr;
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
			size_t indexof() const
			{
				if (!now)
					return treap->size();
				node *now = this->now;
				size_t pos = now->left ? now->left->subtree_size : 0;
				while (now)
				{
					if (now->parent && now->parent->right == now)
					{
						pos += now->parent->left ? now->parent->left->subtree_size + 1 : 1;
					}
					now = now->parent;
				}
				return pos;
			}
			long long operator-(iterator other) const
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
				reheap(now);
			}
		}

		void erase(size_t pos)
		{
			erase(find(pos));
		}

		void erase(iterator it)
		{
			node *now = it.now;
			if (!now->left)
			{
				if (now->parent)
				{
					if (now->parent->left == now)
						now->parent->left = now->right;
					else
						now->parent->right = now->right;
				}
				else
				{
					root = now->right;
				}
				if (now->right)
					now->right->parent = now->parent;
				node *tmp = now->right ? now->right : now->parent;
				delete now;
				while (tmp)
				{
					reheap(tmp);
				}
				return;
			}
			node *right = now->left;
			while (right->right)
			{
				right = right->right;
			}
			right->right = now->right;
			if (now->right)
				now->right->parent = right;
			if (now->parent)
			{
				if (now->parent->left == now)
					now->parent->left = now->left;
				else
					now->parent->right = now->left;
			}
			else
			{
				root = now->left;
			}
			now->left->parent = now->parent;
			delete now;
			while (right)
			{
				reheap(right);
			}
		}

		void merge(size_t pos, Treap &&other)
		{
			if (!root)
			{
				root = other.root;
				other.root = nullptr;
				return;
			}
			if (!other.root)
			{
				return;
			}
			node *now = root;
			if (pos != 0)
			{
				find_node(now, --pos);
				merge_right(now, other.root);
			}
			else
			{
				while (now->left)
				{
					now = now->left;
				}
				now->left = other.root;
				other.root->parent = now;
				while (now)
				{
					reheap(now);
				}
			}
		}

		Treap split(size_t l, size_t r)
		{
			assert(0 <= l && l <= r && r <= size());
			Treap result;
			if (l == r)
			{
				return result;
			}
			result.insert(0, *find(l));
			erase(l++);
			std::vector<node *> erase_nodes;
			while (l != r)
			{
				node *now = root;
				find_node(now, l);
				if (now->subtree_size != 1)
				{
					result.insert(result.size(), now->raw_value);
					erase_nodes.push_back(now);
					l++;
				}
				else
				{
					while (now->parent && now->parent->subtree_size + l <= r)
					{
						now = now->parent;
					}
					result.merge_right(result.root, now);
					l += now->subtree_size;
				}
			}
			for (node *v : erase_nodes)
			{
				erase(iterator(this, v));
			}
			return result;
		}

		T prod(size_t l, size_t r) const
		{
			assert(0 <= l && l <= r && r <= size());
			T val = e();
			while (l != r)
			{
				node *now = root;
				find_node(now, l);
				if (now->subtree_size != 1)
				{
					val = op(val, now->raw_value);
					l++;
					if (now->right && now->right->subtree_size + l <= r)
					{
						val = op(val, now->right->value);
						l += now->right->subtree_size;
					}
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

		T get(size_t pos) const
		{
			node *now = root;
			find_node(now, pos);
			return now->value;
		}

		void set(size_t pos, T value)
		{
			node *now = root;
			find_node(now, pos);
			now->raw_value = value;
			while (now)
			{
				reheap(now);
			}
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

}
