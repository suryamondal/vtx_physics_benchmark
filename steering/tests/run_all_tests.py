#!/usr/bin/env python3
"""Runs all the tests. Assumes all paths are without \ and quotes."""
import os
import subprocess

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# basf2 setup (works on KEKCC)
B2SETUP_PATH = "/cvmfs/belle.cern.ch/sl6/tools/b2setup"
BASF2_MASTER_DIR = "/home/belle2/lmassa/b2devel"  # No central path for master
BASF2_UPGRADE_DIR = "/home/belle2/lmassa/b2vtx"  # No central path for upgrade
SHELL_PATH = "/bin/bash"

# Commands to setup basf2
CMD_SETUP_MASTER = f'cd "{BASF2_MASTER_DIR}"; source "{B2SETUP_PATH}"; cd -'
CMD_SETUP_UPGRADE = f'cd "{BASF2_UPGRADE_DIR}"; source "{B2SETUP_PATH}"; cd -'


def run_test(cmd, log_filename, env, use_upgrade, expected_failure=False):
    setup_cmd = CMD_SETUP_UPGRADE if use_upgrade else CMD_SETUP_MASTER
    cmd_line = f'{setup_cmd}; {cmd}'
    cmd_args = [SHELL_PATH, "-c", cmd_line]
    print("Command:", cmd_line)
    with open(os.path.join("outputs", log_filename), 'w') as ofs:
        rc = subprocess.call(cmd_args, env=env, stdout=ofs,
                             stderr=subprocess.STDOUT)
    print("Exit code:", rc)
    if expected_failure:
        return rc != 0
    return rc == 0


if __name__ == "__main__":
    os.chdir(SCRIPT_DIR)
    if not os.path.isdir("outputs"):
        print("Current directory:", os.getcwd())
        print('Creating "outputs" directory.')
        os.mkdir("outputs")

    with open("outputs/all_tests.log", "w") as log_file:
        def log(*args, **kwargs):
            print(*args, **kwargs)
            print(*args, file=log_file, **kwargs)

        log("Current directory:", os.getcwd())

        env = { k: v for k, v in os.environ.items() }
        # Prevent b2setup from asking private key passphrase
        # (it is necessary to check for updates through git)
        env['BELLE2_NO_TOOLS_CHECK'] = 'y'


        log("\n[TEST] Generate VXD with upgrade (expected failure)")
        res = run_test("./test_vxd_gen.sh", "gen_vxd_upgrade.log", env,
                       use_upgrade=True, expected_failure=True)
        log("[PASSED]" if res else "[NOT PASSED]")

        log("\n[TEST] Generate VXD with master")
        res = run_test("./test_vxd_gen.sh", "gen_vxd_master.log", env,
                       use_upgrade=False, expected_failure=False)
        log("[PASSED]" if res else "[NOT PASSED]")

        log("\n[TEST] Generate VTX with master (expected failure)")
        res = run_test("./test_vtx_gen.sh", "gen_vtx_master.log", env,
                       use_upgrade=False, expected_failure=True)
        log("[PASSED]" if res else "[NOT PASSED]")

        log("\n[TEST] Generate VTX with upgrade")
        res = run_test("./test_vtx_gen.sh", "gen_vtx_upgrade.log", env,
                       use_upgrade=True, expected_failure=False)
        log("[PASSED]" if res else "[NOT PASSED]")


        log("\n[TEST] Analyse VXD with upgrade (expected failure)")
        res = run_test("./test_vxd_ana.sh", "ana_vxd_upgrade.log", env,
                       use_upgrade=True, expected_failure=True)
        log("[PASSED]" if res else "[NOT PASSED]")

        log("\n[TEST] Analyse VXD with master")
        res = run_test("./test_vxd_ana.sh", "ana_vxd_master.log", env,
                       use_upgrade=False, expected_failure=False)
        log("[PASSED]" if res else "[NOT PASSED]")

        log("\n[TEST] Analyse VTX with master (expected failure)")
        res = run_test("./test_vtx_ana.sh", "ana_vtx_master.log", env,
                       use_upgrade=False, expected_failure=True)
        log("[PASSED]" if res else "[NOT PASSED]")

        log("\n[TEST] Analyse VTX with upgrade")
        res = run_test("./test_vtx_ana.sh", "ana_vtx_upgrade.log", env,
                       use_upgrade=True, expected_failure=False)
        log("[PASSED]" if res else "[NOT PASSED]")


        log("\n[TEST] Analyse both with master (expected failure)")
        res = run_test("./test_mix_ana.sh", "ana_mix_master.log", env,
                       use_upgrade=False, expected_failure=True)
        log("[PASSED]" if res else "[NOT PASSED]")

        log("\n[TEST] Analyse both with upgrade (expected failure)")
        res = run_test("./test_mix_ana.sh", "ana_mix_upgrade.log", env,
                       use_upgrade=True, expected_failure=True)
        log("[PASSED]" if res else "[NOT PASSED]")
