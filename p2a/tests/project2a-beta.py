import os, subprocess, shutil

import toolspath
from testing import Test, BuildTest

shell_prompt = "wish> "
call_shell = "wish <"
err_message = "An error has occurred\n"

def readall(filename):
  f = open(filename, 'r')
  s = f.read()
  f.close()
  return s

class ShellBuildTest(BuildTest):
  targets = ['wish']
  point_value = 5

  def run(self):
    self.clean(['wish', '*.o'])
    if not self.make(self.targets):
      self.run_util(['gcc', 'wish.c', '-o', 'wish'])
    self.done()

class ShellTest(Test):

  def run(self, command = None, stdout = None, stderr = None, 
          addl_args = []):
    in_path = self.test_path + '/' + self.name + '/in'
    if command == None:
      command = ['./wish']

    out_path = self.test_path + '/' + self.name + '/out'
    err_path = self.test_path + '/' + self.name + '/err'
    self.command = call_shell + in_path + \
        "\n and check out the test folder\n " + self.test_path \
        + '/' + self.name + \
        "\n to compare your output with reference outputs. "

    # process the input file with input redirection
    stdin = readall(in_path)

    if stdout == None:
      stdout = readall(out_path)
    if stderr == None:
      stderr = readall(err_path)

    self.runexe(command, status=self.status, 
                stdin=stdin, stderr=stderr, stdout=stdout)
    self.done()



######################### Built-in Commands #########################

class Exit(ShellTest):
  name = 'exit'
  description = 'run exit'
  timeout = 10
  status = 0
  point_value = 5



class CD(ShellTest):
  name = 'cd'
  description = 'cd with argument, and cd without argument'
  timeout = 10
  status = 0
  point_value = 5
  def run(self):
    out1 = subprocess.Popen(['/bin/ls', '/u'], 
                             stdout=subprocess.PIPE).communicate()[0]
    #out2 = subprocess.Popen(['/bin/ls', os.environ['HOME']], 
    #                        stdout=subprocess.PIPE).communicate()[0]
    super(CD, self).run(stdout = shell_prompt + 
                        shell_prompt + out1 + 
                        shell_prompt + 
                        shell_prompt + out1 +
                        shell_prompt)
class History(ShellTest):
  name = 'history'
  description = 'test history without argument'
  timeout = 10
  status = 0
  point_value = 5

  
class HistoryArg(ShellTest):
  name = 'historyarg'
  description = 'test history with argument'
  timeout = 10
  status = 0
  point_value = 5
  
class BadCD(ShellTest):
  name = 'badcd'
  description = 'cd to a bad directory'
  timeout = 10
  status = 0
  point_value = 5

class Exec(ShellTest):
  name = 'exec'
  description = 'path should be /bin'
  timeout = 10
  status = 0
  point_value = 5

class BadExec(ShellTest):
  name = 'badexec'
  description = 'run programs that do not exist in /bin'
  timeout = 10
  status = 0
  point_value = 10

class Path(ShellTest):
  name = 'path'
  description = 'run path and a program that exists in a given path'
  timeout = 10
  status = 0
  point_value = 5

class Path2(ShellTest):
  name = 'path2'
  description = 'run path with a long pathname and many different paths'
  timeout = 10
  status = 0
  point_value = 5

class Pip(ShellTest):
  name = 'pip'
  description = 'simple piping'
  timeout = 10
  status = 0
  point_value = 10


######################### Formatting ###########################

class Line(ShellTest):
  name = 'line'
  description = 'line with maximum allowed length'
  timeout = 10
  status = 0
  point_value = 5

class BadLine(ShellTest):
  name = 'badline'
  description = 'a line that is too long'
  timeout = 10
  status = 0
  point_value = 5

class Batch(ShellTest):
  name = 'batch'
  description = 'test batch mode'
  timeout = 10
  status = 1
  def run(self):
    super(BadArg, self).run(command = ['./wish', '/extra'])
  point_value = 5
  
class BadArg(ShellTest):
  name = 'badarg'
  description = 'extra argument to wish'
  timeout = 10
  status = 1
  def run(self):
    super(BadArg, self).run(command = ['./wish', '/extra'])
  point_value = 5

class WhiteSpace(ShellTest):
  name = 'whitespace'
  description = 'leading and trailing whitespace, full whitespaces or empty string'
  timeout = 10
  status = 0
  point_value = 5


######################### Redirection #############################

class Rdr(ShellTest):
  name = 'rdr'
  description = 'simple redirection with no standard error'
  timeout = 10
  status = 0
  point_value = 10
  
  def run(self):
    #project_path = os.getcwd()
    super(Rdr, self).run()
    if os.path.isfile(self.project_path + "/output.out") == False:
      self.fail("missing redirected standard output output.out")
    out = readall(self.project_path + "/output.out")
    ref_out = readall(self.test_path + '/' + self.name + '/output.out')
    if (out != ref_out): self.fail("standard output does not match redirected output")
    
    # if os.path.isfile(project_path + "/output.err") == False:
    #   self.fail("missing redirected standard error output.err")
    # err = readall(project_path + "/output.err")
    # ref_err = readall(self.test_path + '/' + self.name + '/output.err')
    # if (err != ref_err): self.fail("standard error does not match redirected error")
    
    self.done()

class Rdr2(ShellTest):
  name = 'rdr2'
  description = 'simple redirection with standard error'
  timeout = 10
  status = 0
  point_value = 10

  def run(self):
    super(Rdr2, self).run()
    if os.path.isfile(self.project_path + "/output.out") == False:
      self.fail("missing redirected standard output output.out")
    out = readall(self.project_path + "/output.out")
    ref_out = readall(self.test_path + '/' + self.name + '/output.out')
    if (out != ref_out): self.fail("standard output does not match redirected output")
    
    # if os.path.isfile(self.project_path + "/output.err") == False:
    #   self.fail("missing redirected standard error output.err")
    # err = readall(self.project_path + "/output.err")
    # ref_err = readall(self.test_path + '/' + self.name + '/output.err')
    # if (err != ref_err): self.fail("standard error does not match redirected error")
    
    self.done()

class BadRdr(ShellTest):
  name = 'badrdr'
  description = 'bad redirection format'
  timeout = 10
  status = 0
  point_value = 5

class BadRdr2(ShellTest):
  name = 'badrdr2'
  description = 'bad redirection that redirects to invalid path'
  timeout = 10
  status = 0
  point_value = 5

#=========================================================================

all_tests = [
  # Built-in Command
   Exit,
   CD,
   BadCD,
   History,
   HistoryArg,
   Exec,
   BadExec,
   Path,
   Path2,
   Pip,

  # # Formatting
   Line,
   BadArg,
   Batch,
   WhiteSpace,

  # # Redirection
   Rdr,
  ]

build_test = ShellBuildTest

from testing.runtests import main
main(build_test, all_tests)
