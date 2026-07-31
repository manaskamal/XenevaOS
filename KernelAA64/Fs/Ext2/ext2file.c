#include <Fs/vfs.h>
#include <Fs/vdisk.h>
#include <Fs/Ext2/ext2file.h>
#include <aucon.h>


size_t Ext2Write(AuVFSNode* node, AuVFSNode* file, uint64_t* buffer, uint32_t length){
    if (!node || !file || !buffer || !length){
        const char* missing = !node ? "node" : !file ? "file" : !buffer ? "buffer" : "length";
        AuTextOut("[Ext2]: %s parameter missing for file writing.\r\n", missing);
        return 0;
    }
};