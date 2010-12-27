import glob, os
package_dir = os.path.dirname(__file__)
modules_list = glob.glob(package_dir + "/*.py")
init_file = os.path.join(package_dir, "__init__.py")
modules_list = filter((lambda s: s != init_file), modules_list)
module_names = [ os.path.basename(module) for module in modules_list ]
module_names = [ module_name[:-3] for module_name in module_names ]
__all__ = module_names
