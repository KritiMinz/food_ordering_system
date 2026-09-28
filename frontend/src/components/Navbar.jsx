function Navbar({
  cartItemCount,
  onLogin,
  currentUser,
  onLogout,
}) {
  return (
    <nav className="bg-white shadow-md">

      <div className="max-w-7xl mx-auto px-6 py-4 flex items-center justify-between">

        <div className="text-2xl font-bold text-orange-600">
          🍔 FoodHub
        </div>

        <div className="flex items-center gap-8 text-gray-700">

          <a href="#" className="hover:text-orange-600">
            Home
          </a>

          <a href="#menu" className="hover:text-orange-600">
            Menu
          </a>

          <a href="#orders" className="hover:text-orange-600">
            Orders
          </a>

          <a href="#cart" className="hover:text-orange-600">
            Cart 🛒

            {cartItemCount > 0 && (
              <span className="ml-1 bg-orange-600 text-white text-xs
                               px-2 py-1 rounded-full">
                {cartItemCount}
              </span>
            )}
          </a>

          {currentUser ? (
            <div className="flex items-center gap-3">

              <span className="font-semibold text-gray-700">
                👤 {currentUser.name}
              </span>

              <button
                onClick={onLogout}
                className="border border-orange-600 text-orange-600
                           px-4 py-2 rounded-lg
                           hover:bg-orange-50"
              >
                Logout
              </button>

            </div>
          ) : (
            <button
              onClick={onLogin}
              className="bg-orange-600 text-white px-5 py-2 rounded-lg
                         hover:bg-orange-700"
            >
              Login
            </button>
          )}

        </div>

      </div>

    </nav>
  )
}

export default Navbar