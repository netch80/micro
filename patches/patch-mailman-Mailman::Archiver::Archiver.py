--- Mailman/Archiver/Archiver.py
+++ Mailman/Archiver/Archiver.py
@@ -191,24 +191,29 @@
     def ArchiveMail(self, msg):
         """Store postings in mbox and/or pipermail archive, depending."""
         # Fork so archival errors won't disrupt normal list delivery
-        if mm_cfg.ARCHIVE_TO_MBOX == -1:
+        ## !!! Netch: convert it to bit mask (b0 - mbox,
+        ## b1 - usual internal one, b2 - external one)
+        if mm_cfg.ARCHIVE_TO_MBOX in (0, -1):
             return
         #
         # We don't need an extra archiver lock here because we know the list
         # itself must be locked.
-        if mm_cfg.ARCHIVE_TO_MBOX in (1, 2):
+        if mm_cfg.ARCHIVE_TO_MBOX & 1:
             self.__archive_to_mbox(msg)
             if mm_cfg.ARCHIVE_TO_MBOX == 1:
                 # Archive to mbox only.
                 return
         txt = str(msg)
-        # should we use the internal or external archiver?
+        # should we use the internal Or external archiver?
+        with_external = (mm_cfg.ARCHIVE_TO_MBOX & 4)
         private_p = self.archive_private
-        if mm_cfg.PUBLIC_EXTERNAL_ARCHIVER and not private_p:
+        if mm_cfg.PUBLIC_EXTERNAL_ARCHIVER and not private_p \
+                and with_external:
             self.ExternalArchive(mm_cfg.PUBLIC_EXTERNAL_ARCHIVER, txt)
-        elif mm_cfg.PRIVATE_EXTERNAL_ARCHIVER and private_p:
+        elif mm_cfg.PRIVATE_EXTERNAL_ARCHIVER and private_p \
+                and with_external:
             self.ExternalArchive(mm_cfg.PRIVATE_EXTERNAL_ARCHIVER, txt)
-        else:
+        if mm_cfg.ARCHIVE_TO_MBOX & 2:
             # use the internal archiver
             f = StringIO(txt)
             import HyperArch
