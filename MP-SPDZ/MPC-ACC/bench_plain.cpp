#include "AccKeyless.h"
#include "MerkleTree.h"
#include "RCurve.h"
#include "Tools/time-func.h"

#define RUNS 100

bool ACCBenchmark(int set_size) {
  using KEYLESS::Acc;
  using KEYLESS::Auxillary;
  using KEYLESS::Witness;

  Timer timer;

  double gen_time = 0;
  double eval_time = 0;
  double wit_create_time = 0;
  double add_time = 0;
  double wit_update_add_time = 0;
  double delete_time = 0;
  double wit_update_delete_time = 0;

  osuCrypto::REllipticCurve curve(B12_P381, RLC_EP_MTYPE);
  curve.printParam();
  curve.init_field();
  Acc acc_class(curve, set_size + 1);

  Acc::Accumulator acc;
  Auxillary aux;
  Acc::Set el;
  acc_class.genRandomSet(el, set_size);
  Witness wit;
  Acc::Element new_el;
  new_el.randomize();
  Witness old_wit;
  Acc::Wit tmp1;
  Acc::Element tmp2;

  for (int runs = 0; runs < RUNS; runs++)
  {
    std::cout << "-------------------------------------------------" << std::endl;
    std::cout << "Run: " << (runs + 1) << " of " << RUNS << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;

    double tmp_time;

    timer.start();
    acc_class.Gen();
    timer.stop();
    tmp_time = timer.elapsed() * 1e3;
    gen_time += tmp_time;
    std::cout << "Gen time: " << tmp_time << " ms" << std::endl;
    timer.reset();

    timer.start();
    acc_class.Eval(el, acc, aux);
    timer.stop();
    tmp_time = timer.elapsed() * 1e3;
    eval_time += tmp_time;
    std::cout << "Eval time: " << tmp_time << " ms" << std::endl;
    timer.reset();

    timer.start();
    bool ret = acc_class.WitCreate(aux, el[0], wit);
    timer.stop();
    tmp_time = timer.elapsed() * 1e3;
    if(!ret)
        return false;
    wit_create_time += tmp_time;
    std::cout << "WitCreate time: " << tmp_time << " ms" << std::endl;
    timer.reset();

    if (!acc_class.Verify(acc, el[0], wit))
        return false;
    if (acc_class.Verify(acc, el[1], wit))
        return false;

    timer.start();
    ret = acc_class.Add(acc, aux, new_el);
    timer.stop();
    tmp_time = timer.elapsed() * 1e3;
    if (!ret)
        return false;
    add_time += tmp_time;
    std::cout << "Add time: " << tmp_time << " ms" << std::endl;
    timer.reset();

    tmp1 = wit.getWitness();
    tmp2 = wit.getElement();
    old_wit.init(tmp1, tmp2);

    timer.start();
    ret = acc_class.WitUpdate(wit, aux, new_el);
    timer.stop();
    tmp_time = timer.elapsed() * 1e3;
    if (!ret)
        return false;
    wit_update_add_time += tmp_time;
    std::cout << "WitUpdate Add time: " << tmp_time << " ms" << std::endl;
    timer.reset();

    if (!acc_class.Verify(acc, el[0], wit))
        return false;
    if (acc_class.Verify(acc, el[0], old_wit))
        return false;

    timer.start();
    ret = acc_class.Delete(acc, aux, el[2]);
    timer.stop();
    tmp_time = timer.elapsed() * 1e3;
    if (!ret)
        return false;
    delete_time += tmp_time;
    std::cout << "Delete time: " << tmp_time << " ms" << std::endl;
    timer.reset();

    tmp1 = wit.getWitness();
    tmp2 = wit.getElement();
    old_wit.init(tmp1, tmp2);

    timer.start();
    ret = acc_class.WitUpdate(wit, aux, el[2]);
    timer.stop();
    tmp_time = timer.elapsed() * 1e3;
    if (!ret)
        return false;
    wit_update_delete_time += tmp_time;
    std::cout << "WitUpdate Del time: " << tmp_time << " ms" << std::endl;
    timer.reset();

    if (!acc_class.Verify(acc, el[0], wit))
        return false;
    if (acc_class.Verify(acc, el[0], old_wit))
        return false;
  }

  std::cout << "-------------------------------------------------" << std::endl;
  std::cout << "Average for " << RUNS << " runs" << std::endl;
  std::cout << "-------------------------------------------------" << std::endl;
  std::cout << "Gen time: " << gen_time / RUNS << " ms" << std::endl;
  std::cout << "Eval time: " << eval_time / RUNS << " ms" << std::endl;
  std::cout << "WitCreate time: " << wit_create_time / RUNS << " ms" << std::endl;
  std::cout << "Add time: " << add_time / RUNS << " ms" << std::endl;
  std::cout << "WitUpdateAdd time: " << wit_update_add_time / RUNS << " ms" << std::endl;
  std::cout << "Delete time: " << delete_time / RUNS << " ms" << std::endl;
  std::cout << "WitUpdateDel time: " << wit_update_delete_time / RUNS << " ms" << std::endl;
  return true;
}

bool MerkleBenchmark(int set_size) {
  Timer timer;

  double building_tree_time = 0;
  double proof_begin_time = 0;
  double proof_end_time = 0;
  double verify_begin_time = 0;
  double verify_end_time = 0;

  std::cout << "Hashing..." << std::endl;
  timer.start();
  std::vector<MerkleTree::digest_storage> hashes;
  hashes.reserve(set_size);
  SeededPRNG G;
  crypto_hash_sha256_state state;
  osuCrypto::REllipticCurve curve;
  gfp_<100, 4>::init_field(curve.getOrder().toString(10).c_str(), false);
  gfp_<100, 4> bigint;
  for (int i = 0; i < set_size; i++) {
    bigint.randomize(G);
    MerkleTree::digest_storage dig;
    crypto_hash_sha256_init(&state);
    crypto_hash_sha256_update(&state, (unsigned char*)bigint.get_ptr(), bigint.size());
    crypto_hash_sha256_final(&state, dig.data());
    hashes.push_back(std::move(dig));
  }
  timer.stop();
  std::cout << "Hashing time: " << timer.elapsed() * 1e3 << std::endl;
  timer.reset();

  for (int runs = 0; runs < RUNS; runs++)
  {
    std::cout << "-------------------------------------------------" << std::endl;
    std::cout << "Run: " << (runs + 1) << " of " << RUNS << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;

    double tmp_time;

    timer.start();
    MerkleTree tree(hashes);
    timer.stop();
    tmp_time = timer.elapsed() * 1e3;
    building_tree_time += tmp_time;
    std::cout << "Building tree time: " << tmp_time << " ms" << std::endl;
    timer.reset();

    timer.start();
    auto proof_b = tree.proof(hashes.front());
    timer.stop();
    tmp_time = timer.elapsed() * 1e3;
    proof_begin_time += tmp_time;
    std::cout << "Proof Begin time: " << tmp_time << " ms" << std::endl;
    timer.reset();

    timer.start();
    auto proof_e = tree.proof(hashes.back());
    timer.stop();
    tmp_time = timer.elapsed() * 1e3;
    proof_end_time += tmp_time;
    std::cout << "Proof End time: " << tmp_time << " ms" << std::endl;
    timer.reset();

    timer.start();
    bool cvb = tree.verify(hashes.front(), proof_b);
    timer.stop();
    tmp_time = timer.elapsed() * 1e3;
    verify_begin_time += tmp_time;
    std::cout << "Verify Begin time: " << tmp_time << " ms" << std::endl;
    timer.reset();

    timer.start();

    bool cve = tree.verify(hashes.back(), proof_e);
    timer.stop();
    tmp_time = timer.elapsed() * 1e3;
    verify_end_time += tmp_time;
    std::cout << "Verify End time: " << tmp_time << " ms" << std::endl;
    timer.reset();

    if (!cvb || !cve)
      return false;
  }

  std::cout << "-------------------------------------------------" << std::endl;
  std::cout << "Average for " << RUNS << " runs" << std::endl;
  std::cout << "-------------------------------------------------" << std::endl;
  std::cout << "Building tree time: " << building_tree_time / RUNS << " ms" << std::endl;
  std::cout << "Proof Begin time: " << proof_begin_time / RUNS << " ms" << std::endl;
  std::cout << "Proof End time: " << proof_end_time / RUNS << " ms" << std::endl;
  std::cout << "Verify Begin time: " << verify_begin_time / RUNS << " ms" << std::endl;
  std::cout << "Verify End time: " << verify_end_time / RUNS << " ms" << std::endl;
  return true;
}

int main(int argc, const char** argv)
{
  (void)argc;
  (void)argv;
  std::vector<int> set_sizes;
  set_sizes.push_back((1 << 10));
  set_sizes.push_back((1 << 14));

  for (auto& set_size : set_sizes) {

    std::cout << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;
    std::cout << "t-SDH TEST START (" << set_size << ")" << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;
    if (ACCBenchmark(set_size))
      std::cout << "Test passed" << std::endl;
    else
      std::cout << "Test failed..." << std::endl;

    std::cout << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;
    std::cout << "Merkle-tree TEST START (" << set_size << ")" << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;
    if (MerkleBenchmark(set_size))
      std::cout << "Test passed" << std::endl;
    else
      std::cout << "Test failed..." << std::endl;
  }
}
