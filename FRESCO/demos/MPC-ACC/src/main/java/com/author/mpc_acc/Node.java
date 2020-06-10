package com.author.mpc_acc;

import iaik.security.md.SHA256;

public class Node {
  public byte[] digest;
  public Node left;
  public Node right;

  public Node(byte[] digest) {
    this.digest = digest;
    left = null;
    right = null;
  }

  public Node(Node left, Node right) {
    this.left = left;
    this.right = right;
    compute_digest();
  }

  public void compute_digest() {
    if (right != null)
      digest = compute_digest(left.digest, right.digest);
    else
      digest = compute_digest(left.digest, left.digest);
  }

  public static byte[] compute_digest(byte[] left, byte[] right) {
    SHA256 sha256 = new SHA256();
    sha256.update(left);
    sha256.update(right);
    return sha256.digest();
  }
}
