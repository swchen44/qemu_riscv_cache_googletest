"""Read-only verifier contract tests; no apt, network, credentials or root use."""
import hashlib,json,pathlib,subprocess,sys,tempfile,unittest
SCRIPT=pathlib.Path(__file__).resolve().parents[1]/'scripts/ubuntu/verify-bundle.py'
class BundleVerifierTests(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup)
  self.root=pathlib.Path(self.tmp.name)/'bundle';self.root.mkdir()
  f=self.root/'apt/archives/example.deb';f.parent.mkdir(parents=True);f.write_bytes(b'fixture bytes, not an executable package')
  self.item={'path':'apt/archives/example.deb','sha256':hashlib.sha256(f.read_bytes()).hexdigest()}
  self.write_lock(self.item)
 def write_lock(self,item):
  (self.root/'ubuntu-packages.lock.json').write_text(json.dumps({'packages':[item],'probe_packages':[]}))
 def run_verifier(self,*args):
  return subprocess.run([sys.executable,str(SCRIPT),*args],capture_output=True,text=True)
 def test_explicit_root_passes(self):
  self.assertEqual(0,self.run_verifier('--bundle-root',str(self.root)).returncode)
 def test_missing_root_rejected(self):
  self.assertNotEqual(0,self.run_verifier().returncode)
 def test_changed_required_bytes_rejected(self):
  (self.root/self.item['path']).write_bytes(b'changed')
  self.assertNotEqual(0,self.run_verifier('--bundle-root',str(self.root)).returncode)
 def test_path_escape_rejected(self):
  outside=self.root.parent/'outside.deb';outside.write_bytes(b'outside')
  self.write_lock({'path':'../outside.deb','sha256':hashlib.sha256(outside.read_bytes()).hexdigest()})
  self.assertNotEqual(0,self.run_verifier('--bundle-root',str(self.root)).returncode)
 def test_unlisted_extra_file_not_required(self):
  (self.root/'apt/archives/stray.deb').write_bytes(b'not in manifest')
  result=self.run_verifier('--bundle-root',str(self.root));self.assertEqual(0,result.returncode)
  self.assertIn('1 Ubuntu package files',result.stdout)
if __name__=='__main__':unittest.main()
