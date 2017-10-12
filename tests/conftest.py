import os
import pytest
import subprocess
import shlex

class Utils(object):
  def __init__(self):
    self.root_path = os.getcwd()
    self.path = None

  def set_path(self, path):
    self.path = os.path.dirname(path)

  def do_cmd(self, cmd, expected=0, do_assert=True):
    cmd = "cd {}; {}".format(self.path, cmd)
    print("cmd: {}".format(cmd))
    ret = os.system(cmd)
    if do_assert:
      assert ret == expected
    return ret

  def compile(self, test_path):
    self.set_path(test_path)

    # Clear gcda files
    self.do_cmd("rm -f *.gcda *.gcno")

    # Compile
    glib_libs = subprocess.check_output(shlex.split("pkg-config --libs glib-2.0")).strip()
    glib_cflags = subprocess.check_output(shlex.split("pkg-config --cflags glib-2.0")).strip()
    g_lock_manager = "{}/g_lock_manager.c".format(self.root_path)
    self.do_cmd("gcc -g -Wall -std=gnu99 "
      "-fprofile-arcs -ftest-coverage "
      "{cflags} "
      "{g_lock_manager} "
      "main.c "
      "{libs} "
      "-o main".format(
        libs=glib_libs,
        cflags=glib_cflags,
        g_lock_manager=g_lock_manager))

  def run(self, expected=0, signal=None):
    ret = self.do_cmd("./main", expected=expected, do_assert=False)
    if signal is None:
      assert ret == expected
      return

    # Verify the return was a given signal
    exit_status = os.WEXITSTATUS(ret)
    assert exit_status
    print("run exit_status: {}".format(exit_status))

    exit_sig = os.WTERMSIG(exit_status)
    print("run: exit_sig: {} expected: {}".format(exit_sig, signal))
    assert exit_sig == signal

  def compile_and_run(self, path, expected=0, signal=None):
    self.compile(path)
    self.run(expected=expected, signal=signal)

@pytest.yield_fixture
def utils():
  obj = Utils()
  yield obj
