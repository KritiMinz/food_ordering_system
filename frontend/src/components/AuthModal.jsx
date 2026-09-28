function AuthModal({ mode, onClose, onSwitchMode }) {
  const isLogin = mode === "login"

  return (
    <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 px-4">

      <div className="bg-white rounded-2xl shadow-xl w-full max-w-md p-8">

        <div className="flex justify-between items-center mb-6">
          <h2 className="text-3xl font-bold text-gray-800">
            {isLogin ? "Welcome Back" : "Create Account"}
          </h2>

          <button
            onClick={onClose}
            className="text-gray-500 text-2xl hover:text-gray-800"
          >
            ×
          </button>
        </div>

        {!isLogin && (
          <>
            <label className="block text-sm font-semibold text-gray-700 mb-2">
              Full Name
            </label>

            <input
              type="text"
              placeholder="Enter your name"
              className="w-full border rounded-lg px-4 py-3 mb-4"
            />

            <label className="block text-sm font-semibold text-gray-700 mb-2">
              Phone
            </label>

            <input
              type="tel"
              placeholder="Enter phone number"
              className="w-full border rounded-lg px-4 py-3 mb-4"
            />
          </>
        )}

        <label className="block text-sm font-semibold text-gray-700 mb-2">
          Email
        </label>

        <input
          type="email"
          placeholder="Enter your email"
          className="w-full border rounded-lg px-4 py-3 mb-4"
        />

        <label className="block text-sm font-semibold text-gray-700 mb-2">
          Password
        </label>

        <input
          type="password"
          placeholder="Enter password"
          className="w-full border rounded-lg px-4 py-3 mb-4"
        />

        {!isLogin && (
          <>
            <label className="block text-sm font-semibold text-gray-700 mb-2">
              Confirm Password
            </label>

            <input
              type="password"
              placeholder="Confirm password"
              className="w-full border rounded-lg px-4 py-3 mb-5"
            />
          </>
        )}

        <button
          className="w-full bg-orange-600 text-white py-3 rounded-lg
                     font-semibold hover:bg-orange-700"
        >
          {isLogin ? "Login" : "Create Account"}
        </button>

        <p className="text-center text-gray-500 mt-5">
          {isLogin
            ? "Don't have an account?"
            : "Already have an account?"}

          <button
            onClick={onSwitchMode}
            className="text-orange-600 font-semibold ml-2 hover:underline"
          >
            {isLogin ? "Register" : "Login"}
          </button>
        </p>

      </div>

    </div>
  )
}

export default AuthModal