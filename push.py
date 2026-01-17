import subprocess
import sys

def run_command(command):
    """
    执行 Shell 命令并返回输出。
    如果命令执行失败（返回码非0），则打印错误信息并退出脚本。
    """
    try:
        # 使用 shell=True 允许执行完整的命令字符串
        # encoding='utf-8' 适配大多数 Git 输出，errors='replace' 防止编码错误导致崩溃
        result = subprocess.run(
            command, 
            shell=True, 
            check=True, 
            stdout=subprocess.PIPE, 
            stderr=subprocess.PIPE, 
            text=True,
            encoding='utf-8',
            errors='replace'
        )
        return result.stdout.strip()
    except subprocess.CalledProcessError as e:
        print(f"执行命令失败: {command}")
        print(f"错误信息: {e.stderr}")
        sys.exit(1)

def check_git_status():
    """检查当前仓库是否有未提交的更改"""
    print(">> 1. 正在检查 Git 状态...")
    # --porcelain 输出简洁的机器可读格式，如果有输出说明有变动
    status_output = run_command("git status --porcelain")
    
    if status_output:
        print("错误: 当前仓库有未提交的更改 (未 add 或未 commit)。")
        print("请先提交更改后再运行此脚本。")
        print("-" * 30)
        print(status_output)
        print("-" * 30)
        sys.exit(1)
    print("Git 状态干净，可以继续。")

def get_remotes():
    """解析 git remote -v 获取远程仓库列表"""
    print(">> 2. 正在获取远程仓库列表...")
    output = run_command("git remote -v")
    remotes = {}
    
    # 输出格式示例:
    # onedrive  F:\OneDrive\GitRepo\FindVEF-Qt6.bundle (fetch)
    # onedrive  F:\OneDrive\GitRepo\FindVEF-Qt6.bundle (push)
    
    for line in output.splitlines():
        parts = line.split()
        if len(parts) >= 2:
            name = parts[0]
            url = parts[1]
            # 优先记录 (push) 的 URL，如果没有标识则直接记录
            # 字典会自动去重，确保每个 remote name 只处理一次
            if "(push)" in line:
                remotes[name] = url
            elif name not in remotes:
                remotes[name] = url
    return remotes

def main():
    # 1. 检查状态
    check_git_status()
    
    # 2. 获取远程仓库
    remotes = get_remotes()
    if not remotes:
        print("未找到任何远程仓库。")
        return

    print(f"找到 {len(remotes)} 个远程仓库，开始处理...")

    # 3. 遍历并推送
    for name, url in remotes.items():
        print(f"\n>> 正在处理远程仓库: [{name}]")
        print(f"   URL: {url}")
        
        # 判断是否为 bundle 文件 (以 .bundle 结尾，忽略大小写)
        if url.lower().endswith(".bundle"):
            print(f"   [类型识别]: Bundle 文件")
            # git bundle create 直接接文件路径，不需要 remote name
            # 使用 --all 备份所有分支和标签
            cmd = f'git bundle create "{url}" --all'
            print(f"   [执行命令]: {cmd}")
            run_command(cmd)
            print("   [结果]: Bundle 创建/更新成功。")
            
        # 判断是否为 git 仓库 (以 .git 结尾，忽略大小写)
        elif url.lower().endswith(".git"):
            print(f"   [类型识别]: Git 仓库")
            # 使用 --force 强制推送
            # 建议加上 --all 以确保推送到所有分支（与 bundle --all 行为一致）
            cmd = f'git push {name} --all --force'
            print(f"   [执行命令]: {cmd}")
            run_command(cmd)
            print("   [结果]: Git Push 推送成功。")
            
        else:
            print(f"   [警告]: 无法识别 URL 类型 (非 .git 也非 .bundle)，跳过处理。")

    print("\n所有操作已完成。")

if __name__ == "__main__":
    main()