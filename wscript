import os

srcdir = os.path.abspath('.')
blddir = 'build'
target = 'iptrie'
VERSION = '0.0.1'

def set_options(opt):
    opt.tool_options('compiler_cxx')

def configure(conf):
    conf.check_tool('compiler_cxx')
    conf.check_tool('node_addon')

def build(context, target=target):
    obj = context.new_task_gen('cxx', 'shlib', 'node_addon')

    obj.source = ['src/iptrie.cc','src/btrie.C']
    obj.target = target

    context.add_post_fun(move_addon)

def move_addon(context, target=target):
    """Move the compiled addon to the local lib directory. Previously compiled
    versions of the addon are overwritten.

    """
    filename = '%s.node' % target
    from_path = os.path.join(srcdir, blddir, 'default', filename)
    to_path = os.path.join(srcdir, 'lib', filename)

    if not os.fs.exists(os.path.join(srcdir, 'lib')):
        os.mkdir(os.path.join(srcdir, 'lib'))
    if os.fs.exists(from_path):
        os.rename(from_path, to_path)
