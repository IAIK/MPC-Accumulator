#pragma once

#include <vector>
#include <memory>
#include <sodium.h>

/// A Merkle tree
class MerkleTree
{
public:
  typedef std::array<uint8_t, crypto_hash_sha256_BYTES> digest_storage;

private:
  struct node
  {
    digest_storage digest_;
    std::shared_ptr<node> left_, right_;

    node(const digest_storage& digest = digest_storage());
    node(const std::shared_ptr<node>& left,
         const std::shared_ptr<node>& right = std::shared_ptr<node>());

    const digest_storage& digest() const
    {
      return digest_;
    }

    void compute_digest();
    void update_seed(const digest_storage& digest);
  };

  std::shared_ptr<node> root_;

public:
  enum position
  {
    left,
    right
  };
  struct proof_node
  {
    digest_storage digest;
    position pos;
  };

  /// Initialize Merkle tree from digests.
  ///
  /// Given the vector of hashes, builds a Merkle tree where the hashes are
  /// placed in the the leaf nodes.
  ///
  /// @param hashes hashes in the leaf nodes
  MerkleTree(const std::vector<digest_storage>& hashes);
  /// Initialize Merkle tree from a root hash.
  ///
  /// In this configuration, the Merkle tree can only be used for verification.
  ///
  /// @param root_hash the root hash
  MerkleTree(const digest_storage& root_hash);

  /// Create a member ship proof for the given digest.
  ///
  /// A proof consists of a sequence of proof_node instances, where each
  /// proof_node declares if the proven value is the left or right input to the
  /// hash function and contains the digest of the sibling.
  ///
  /// @param value digest to proof
  /// @return sequence of proof nodes or an empty sequence of the value is not
  /// contained in the tree
  std::vector<proof_node> proof(const digest_storage& value) const;
  /// Verify the membership of a given value and its proof against the root hash.
  ///
  /// @param value value to be tested
  /// @param proof proof for the given value
  /// @return true if the value is contained in the tree, i.e. the proof matches
  /// the root hash
  bool verify(const digest_storage& value, const std::vector<proof_node>& proof) const;

  /// Return root hash of the Merkle tree
  ///
  /// @return root hash
  digest_storage root_hash() const;

  void update_seed(const digest_storage& digest);

  static digest_storage compute_digest(const digest_storage& left,
                                       const digest_storage& right);
};

template<typename T, typename... Args>
  std::shared_ptr<T> make_shared(Args&&... args) {
      return std::shared_ptr<T>(new T(std::forward<Args>(args)...));
  }
