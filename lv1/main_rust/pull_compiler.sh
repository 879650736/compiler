#!/bin/bash


DATE=$(date +"%Y-%m-%d %H:%M:%S")
COMMIT_MESSAGE="Update compiler files at $DATE"

# 添加更改到 Git
git add * || { echo "Failed to add changes to git"; exit 1; }

# 提交更改
git commit -m "$COMMIT_MESSAGE" || { echo "Failed to commit changes"; exit 1; }

# 推送更改到远程仓库
git push -u origin master || { echo "Failed to push changes to remote repository"; exit 1; }

echo "Update completed successfully."
