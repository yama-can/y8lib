# Treap

## 定義

```cpp
template <typename T, T (*op)(T, T), T (*e)(), typename RandomEngine = mwc256xxa64>
class Treap
{
public:
	Treap();
	void insert(size_t pos, T value);
	void erase(size_t pos);
	void erase(iterator it);
	T prod(size_t l, size_t r) const;
	void merge(size_t pos, Treap &&other);
	Treap split(size_t l, size_t r);
	iterator begin();
	iterator end();
	iterator find(size_t pos);
	size_t size() const;

	class iterator
	{
		Treap *treap;
		node *now;
		iterator(Treap *treap, node *now);
		friend class Treap;

	public:
		iterator() = delete;
		iterator(const iterator &other) = default;
		iterator &operator=(const iterator &other) = default;
		iterator(iterator &&other) = default;
		iterator &operator=(iterator &&other) = default;
		~iterator() = default;
		iterator &operator++();
		iterator operator++(int);
		iterator &operator--();
		iterator operator--(int);
		T &operator*();
		const T &operator*() const;
		T *operator->();
		const T *operator->() const;
		bool operator==(const iterator &other) const;
		bool operator!=(const iterator &other) const;
		size_t indexof() const;
		long long operator-(iterator other) const;
	};
};
```

## 説明

Treap はランダムな値を各頂点に割り当て、その値を元にヒープを作ることにより確率的に平衡が保たれる平衡二分探索木です。

## 制約

- $id(a, e()) = a$
- $id(e(), a) = a$
- $id(a, id(b, c)) = id(id(a, b), c)$

## 使用方法

### $insert(pos, value)$

$pos$ 番目と $pos + 1$ 番目の間に $value$ を挿入します。
ただし、 $pos = size()$ のときは、最後に $value$ を挿入します。

計算量： $\mathcal O(\log size())$

### $erase(pos)$

$pos$ 番目の要素を削除します。

計算量： $\mathcal O(\log size())$

### $prod(l, r)$

$prod(l, r)$ を実行すると、 $op(op(op(op(A_l, A_{l + 1}), A_{l + 2})\cdots A_{r-2}), A_r)$ を計算し、その値を返します。

計算量： $\mathcal O(\log^2 size())$

### $size()$

計算量： $\mathcal O(1)$

## $split(l, r)$

新しく $[l, r)$ の範囲を含む Treap オブジェクトを作成します。
また、元の Treap オブジェクトの内容は $[0, l)$ と $[r, N)$ をつなげたものになります。

## $merge(pos, other)$

Treap オブジェクト $other$ を $other_0$ が $pos$ 番目になるように Treap オブジェクトに追加します。
`std::move` をしてから $other$ を渡す必要があります。
実行後の $other$ の内容は未定義です。
