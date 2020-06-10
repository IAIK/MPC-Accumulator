package com.author.mpc_acc;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Stack;

import iaik.security.ec.math.common.Pair;
import iaik.security.md.SHA256;

/**
 * A simple class for Merkle Trees
 *
 * @author ******
 */
public class MerkleTree {


  public class ProofNode {
    public byte[] digest;
    public Position pos;

    public ProofNode(byte[] digest, Position pos) {
      this.digest = digest;
      this.pos = pos;
    }
  }

  public enum Position {
    left,
    right
  }

  /// the root node of the merkle tree
  private Node root;

  private static int round_up_to_power_of_2(int value)
  {
    int highestOneBit = Integer.highestOneBit(value);
    if (value == highestOneBit) {
        return value;
    }
    return highestOneBit << 1;
  }

  /// Initialize Merkle tree from a set of digests.
  ///
  /// Given the vector of digest, builds a Merkle tree where the digests are placed in the the leaf nodes.
  ///
  /// @param hashes set of digests, which will be hashed into the leaf nodes
  public MerkleTree(List<byte[]> hashes) {
    if (hashes.isEmpty())
      return;

    List<Node> nodes = new ArrayList<>();
    List<Node> new_nodes = new ArrayList<>();
    for (byte[] hash : hashes) {
      nodes.add(new Node(hash.clone()));
    }

    int size = hashes.size();
    int bound = round_up_to_power_of_2(size);
    for (int s = size; s < bound; s++)
      nodes.add(new Node(hashes.get(size -  1).clone()));

    while (nodes.size() > 1)
    {
      new_nodes.clear();
      for (int s = 0; s < nodes.size() - 1; s += 2)
        new_nodes.add(new Node(nodes.get(s), nodes.get(s + 1)));

      // swap
      List<Node> sw = nodes;
      nodes = new_nodes;
      new_nodes = sw;
    }

    root = nodes.get(0);
  }

  /// Initialize Merkle tree from a root hash.
  ///
  /// In this configuration, the Merkle tree can only be used for verification.
  ///
  /// @param root_hash the root hash
  public MerkleTree(byte[] root_hash) {
    this.root = new Node(root_hash);
  }

  private Boolean find(Node node, Stack<Pair<Position, Node>> nodes, byte[] digest) {
    if (node.left != null) {
      nodes.push(Pair.newInstance(Position.left, node.left));
      if (find(node.left, nodes, digest))
        return true;
      nodes.pop();

      if (node.right != null) {
        nodes.push(Pair.newInstance(Position.right, node.right));
        if (find(node.right, nodes, digest))
          return true;
        nodes.pop();
      }
      return false;
    }
    return Arrays.equals(digest, node.digest);
  }

  /// Create a member ship proof for the given digest.
  ///
  /// A proof consists of a sequence of proof_node instances, where each
  /// proof_node declares if the proven value is the left or right input to the
  /// hash function and contains the digest of the sibling.
  ///
  /// @param digest digest to proof
  /// @return sequence of proof nodes or an empty sequence of the value is not
  /// contained in the tree
 public List<ProofNode> proof(byte[] digest) {
    Stack<Pair<Position, Node>> nodes = new Stack<>();

    Boolean found = find(root, nodes, digest);
    List<ProofNode> result = new ArrayList<>();
    if (!found)
      return result;

    while (nodes.size() != 0)
    {
      Pair<Position, Node> pos_and_node = nodes.pop();

      Node parent;
      if (nodes.empty())
        parent = root;
      else
        parent = nodes.lastElement().getSecond();

      Position pos = pos_and_node.getFirst();

      if (pos == Position.left)
        result.add(new ProofNode(parent.right.digest, pos));
      else
        result.add(new ProofNode(parent.left.digest, pos));
    }

    return result;
  }

  /// Verify the membership of a given value and its proof against the root hash.
  ///
  /// @param element value to be tested
  /// @param proof proof for the given value
  /// @return true if the value is contained in the tree, i.e. the proof matches
  /// the root hash
  public Boolean verify(byte[] element, List<ProofNode> proof) {
    byte[] digest = element;

    for (ProofNode pn : proof)
    {
      switch (pn.pos)
      {
        case left:
          digest = Node.compute_digest(digest, pn.digest);
          break;
        case right:
          digest = Node.compute_digest(pn.digest, digest);
          break;
      }
    }

    return Arrays.equals(digest, getRootHash());
  }

  /// Return root hash of the Merkle tree
  ///
  /// @return root hash
  public byte[] getRootHash() {
    return root.digest;
  }

  public static void test(List<BigInteger> X) {
    System.out.println("--------------------------------------------");
    System.out.println("Merkle Tree Test:");
    System.out.println("Hashing...");
    long time = System.currentTimeMillis();
    List<byte[]> hashes = new ArrayList<>();
    for (BigInteger x : X) {
      SHA256 sha256 = new SHA256();
      sha256.update(x.toByteArray());
      hashes.add(sha256.digest());
    }
    time = System.currentTimeMillis() - time;
    System.out.println("Hashing time: " + time + "ms");

    time = System.currentTimeMillis();
    MerkleTree mk = new MerkleTree(hashes);
    time = System.currentTimeMillis() - time;
    System.out.println("Building tree time: " + time + "ms");

    time = System.currentTimeMillis();
    List<ProofNode> proof = mk.proof(hashes.get(0));
    time = System.currentTimeMillis() - time;
    System.out.println("Proof time: " + time + "ms");

    time = System.currentTimeMillis();
    Boolean cv = mk.verify(hashes.get(0), proof);
    time = System.currentTimeMillis() - time;
    System.out.println("Verify time: " + time + "ms");

    time = System.currentTimeMillis();
    Boolean iv = mk.verify(hashes.get(1), proof);
    time = System.currentTimeMillis() - time;
    System.out.println("Verify time: " + time + "ms");

    System.out.println("Correct verify: " + cv);
    System.out.println("Incorrect verify: " +  iv);
  }

}
