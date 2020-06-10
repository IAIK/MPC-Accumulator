#include "MerkleTree.h"

#include <climits>
#include <cmath>
#include <functional>
#include <map>
#include <stack>
#include <type_traits>

namespace
{
  template <class T>
  T round_up_to_power_of_2(T v)
  {
    static_assert(std::is_unsigned<T>::value, "Only works for unsigned types");
    v--;
    for (std::size_t i = 1; i < sizeof(v) * CHAR_BIT; i *= 2)
    {
      v |= v >> i;
    }
    return ++v;
  }
} // namespace

MerkleTree::node::node(const digest_storage& digest) : digest_(digest) {}

MerkleTree::node::node(const std::shared_ptr<MerkleTree::node>& left,
                        const std::shared_ptr<MerkleTree::node>& right)
  : left_(left), right_(right)
{
  compute_digest();
}

void MerkleTree::node::compute_digest()
{
  if (right_)
    digest_ = MerkleTree::compute_digest(left_->digest(), right_->digest());
  else
    digest_ = MerkleTree::compute_digest(left_->digest(), left_->digest());
}

void MerkleTree::node::update_seed(const digest_storage& digest)
{
  if (left_)
  {
    left_->update_seed(digest);
    compute_digest();
  }
  else
    digest_ = digest;
}

MerkleTree::MerkleTree(const std::vector<digest_storage>& hashes)
{
  if (!hashes.size())
    return;

  std::vector<std::shared_ptr<node>> nodes, new_nodes;
  const auto bound = round_up_to_power_of_2(hashes.size());
  nodes.reserve(bound);
  for (const auto& hash : hashes)
    nodes.push_back(std::make_shared<node>(hash));


  for (std::size_t s = hashes.size(); s < bound; ++s)
    nodes.push_back(nodes[hashes.size() - 1]);

  while (nodes.size() > 1)
  {
    new_nodes.clear();
    for (std::size_t s = 0; s < nodes.size() - 1; s += 2)
      new_nodes.push_back(std::make_shared<node>(nodes[s], nodes[s + 1]));
    if (nodes.size() % 2)
      new_nodes.push_back(std::make_shared<node>(nodes[nodes.size() - 1]));
    std::swap(nodes, new_nodes);
  }

  root_ = nodes[0];
}

MerkleTree::MerkleTree(const digest_storage& root_hash)
  : root_(std::make_shared<node>(root_hash))
{
}

std::vector<MerkleTree::proof_node> MerkleTree::proof(const digest_storage& value) const
{
  std::stack<std::pair<position, std::shared_ptr<node>>> nodes;

  const std::function<bool(const std::shared_ptr<node>&)> cb =
      [&nodes, &cb, &value](const std::shared_ptr<node>& node) -> bool {
    if (node->left_)
    {
      nodes.push(std::make_pair(left, node->left_));
      if (cb(node->left_))
        return true;
      nodes.pop();

      if (node->right_)
      {
        nodes.push(std::make_pair(right, node->right_));
        if (cb(node->right_))
          return true;
        nodes.pop();
      }
      return false;
    }

    const bool eq = node->digest_ == value;
    return eq;
  };

  const bool found = cb(root_);
  std::vector<proof_node> result;
  if (!found)
    return result;

  while (nodes.size())
  {
    const auto pos_and_node = nodes.top();
    nodes.pop();

    std::shared_ptr<node> parent;
    if (nodes.empty())
      parent = root_;
    else
      parent = nodes.top().second;

    proof_node pn;
    pn.pos = pos_and_node.first;
    switch (pos_and_node.first)
    {
    case left:
      pn.digest = parent->right_->digest_;
      break;
    case right:
      pn.digest = parent->left_->digest_;
      break;
    }

    result.push_back(pn);
  }

  return result;
}

bool MerkleTree::verify(const digest_storage& value,
                         const std::vector<proof_node>& proof) const
{
  digest_storage digest = value;
  for (const proof_node& pn : proof)
  {
    switch (pn.pos)
    {
    case left:
      digest = compute_digest(digest, pn.digest);
      break;
    case right:
      digest = compute_digest(pn.digest, digest);
      break;
    }
  }

  return digest == root_hash();
}

MerkleTree::digest_storage MerkleTree::root_hash() const
{
  return root_->digest();
}

void MerkleTree::update_seed(const digest_storage& digest)
{
  root_->update_seed(digest);
}

MerkleTree::digest_storage MerkleTree::compute_digest(
                                   const digest_storage& left,
                                   const digest_storage& right)
{
  digest_storage dig;
  crypto_hash_sha256_state state;
  crypto_hash_sha256_init(&state);

  crypto_hash_sha256_update(&state, left.data(), left.size());
  crypto_hash_sha256_update(&state, right.data(), right.size());
  crypto_hash_sha256_final(&state, dig.data());
  return dig;
}
