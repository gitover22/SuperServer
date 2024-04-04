#! /bin/bash
# 简易推送远程库脚本

# check para
if [ $# -ne 1 ]; then
    echo "Usage: $0 <commit_message>"
    exit 1
fi

# add
git add .

# commit
git commit -m "$1"

# push
git push 
