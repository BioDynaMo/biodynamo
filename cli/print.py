class Print:
    PURPLE = '\033[95m'
    CYAN = '\033[96m'
    DARKCYAN = '\033[36m'
    BLUE = '\033[94m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    END = '\033[0m'

    @staticmethod
    def success(message):
        print(Print.BOLD + Print.GREEN + str(message) + Print.END)

    @staticmethod
    def error(message):
        print(Print.RED + str(message) + Print.END)

    @staticmethod
    def warning(message):
        print(Print.YELLOW + str(message) + Print.END)

    @staticmethod
    def new_step(message):
        print('\n' + Print.BOLD + Print.BLUE + str(message) + Print.END)
