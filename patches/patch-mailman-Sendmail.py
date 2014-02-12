--- Mailman/Handlers/Sendmail.py
+++ Mailman/Handlers/Sendmail.py
@@ -41,6 +41,7 @@
 
 import string
 import os
+import re
 
 from Mailman import mm_cfg
 from Mailman import Errors
@@ -48,6 +49,38 @@
 
 MAX_CMDLINE = 3000
 
+
+## Precompile patterns for quote_for_shell() usage.
+rec_regular = re.compile('^[A-Za-z0-9+,.\/\@:=^_-]+$')
+rec_not_for_single = re.compile(r"['\\]")
+rec_not_for_double = re.compile(r'["\\\$]')
+
+## Prepare character map if no simple quoting is allowed.
+qfs_map = {}
+for c in [chr(x) for x in range(1, 256)]:
+    if rec_regular.search(c):
+        qfs_map[c] = c
+    else:
+        qfs_map[c] = '\\' + c
+
+
+def quote_for_shell(arg):
+    if arg is None:
+        return ''
+    if '\0' in arg:
+        ## Hmm, must not happen... Don't allow this argument at all.
+        return ''
+    ## Sequentially test variants from a trivial to the most complex one.
+    if arg == '':
+        return '""'
+    if rec_regular.match(arg):
+        return arg
+    if not rec_not_for_single.search(arg):
+        return "'" + arg + "'"
+    if not rec_not_for_double.search(arg):
+        return '"' + arg + '"'
+    return ''.join(qfs_map[c] for c in arg)
+
 
 
 def process(mlist, msg, msgdata):
@@ -68,27 +101,27 @@
     # WARN: If you've read the warnings above and /still/ insist on using this
     # module, you must comment out the following line.  I still recommend you
     # don't do this!
-    assert 0, 'Use of the Sendmail.py delivery module is highly discouraged'
     recips = msgdata.get('recips')
     if not recips:
         # Nobody to deliver to!
         return
     # Use -f to set the envelope sender
-    cmd = mm_cfg.SENDMAIL_CMD + ' -f ' + mlist.GetBouncesEmail() + ' '
+    cmd = mm_cfg.SENDMAIL_CMD + ' -f ' + mlist.GetBouncesEmail() + ' -- '
     # make sure the command line is of a manageable size
     recipchunks = []
     currentchunk = []
     chunklen = 0
     for r in recips:
-        currentchunk.append(r)
-        chunklen = chunklen + len(r) + 1
+        qr = quote_for_shell(r)
+        currentchunk.append(qr)
+        chunklen = chunklen + len(qr) + 1
         if chunklen > MAX_CMDLINE:
-            recipchunks.append(string.join(currentchunk))
+            recipchunks.append(' '.join(currentchunk))
             currentchunk = []
             chunklen = 0
     # pick up the last one
     if chunklen:
-        recipchunks.append(string.join(currentchunk))
+        recipchunks.append(' '.join(currentchunk))
     # get all the lines of the message, since we're going to do this over and
     # over again
     msgtext = str(msg)
